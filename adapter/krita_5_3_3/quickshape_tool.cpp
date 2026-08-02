#include "quickshape_tool.hpp"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_cursor.h>
#include <kis_global.h>
#include <kis_painting_information_builder.h>
#include <kis_tool_freehand_helper.h>
#include <klocalizedstring.h>
#include <kundo2magicstring.h>
#include <KoIcon.h>

#include <cmath>

namespace {
constexpr int kEndpointHoldMilliseconds = 600;
constexpr double kMaximumEndpointDriftPixels = 2.0;
}

class QuickShapeTool::FreehandHelper final : public KisToolFreehandHelper {
public:
    FreehandHelper(KisPaintingInformationBuilder* infoBuilder,
                   KoCanvasResourceProvider* resourceManager)
        : KisToolFreehandHelper(infoBuilder,
                                resourceManager,
                                kundo2_i18n("QuickShape Stroke"),
                                new KisSmoothingOptions(false)) {}

    void cancelRunningStroke() { cancelPaint(); }

    bool beginEndpointReplay(const quickshape::Sample& sample,
                             KisImageWSP image,
                             KisNodeSP node,
                             KisStrokesFacade* strokesFacade) {
        if (!image || !node || !strokesFacade || isRunning()) return false;

        KisPaintInformation endpoint(
            QPointF(sample.position.x, sample.position.y),
            sample.pressure,
            sample.tilt_x,
            sample.tilt_y,
            sample.rotation,
            sample.tangential_pressure,
            1.0,
            0.0,
            0.0);
        initPaintImpl(0.0, endpoint, resourceManager(), image, node, strokesFacade);
        paintLine(endpoint, endpoint);
        return isRunning();
    }
};

QuickShapeTool::QuickShapeTool(KoCanvasBase* canvas)
    : KisToolFreehand(canvas,
                      KisCursor::load("tool_freehand_cursor.xpm", 2, 2),
                      kundo2_i18n("QuickShape Stroke"),
                      false) {
    setObjectName("quickshape_tool");
    helper_ = new FreehandHelper(paintingInformationBuilder(),
                                 canvas->resourceManager());
    resetHelper(helper_);
    holdTimer_.setSingleShot(true);
    holdTimer_.setInterval(kEndpointHoldMilliseconds);
    connect(&holdTimer_, &QTimer::timeout,
            this, &QuickShapeTool::qualifyEndpointHold);
}

quickshape::Sample QuickShapeTool::sampleFromEvent(
    const KoPointerEvent* event) {
    const QPointF imagePoint = convertToPixelCoord(event->point);
    return {
        .position = {imagePoint.x(), imagePoint.y()},
        .timestamp_us = strokeClock_.isValid()
            ? strokeClock_.nsecsElapsed() / 1000
            : 0,
        .pressure = event->pressure(),
        .tilt_x = event->xTilt(),
        .tilt_y = event->yTilt(),
        .rotation = event->rotation(),
        .tangential_pressure = event->tangentialPressure(),
    };
}

void QuickShapeTool::beginPrimaryAction(KoPointerEvent* event) {
    strokeClock_.restart();
    const auto sample = sampleFromEvent(event);
    lifecycle_.finish();
    const bool captureStarted = lifecycle_.begin(sample);
    KIS_SAFE_ASSERT_RECOVER_RETURN(captureStarted);
    holdAnchor_ = sample.position;
    replacementRunning_ = false;

    KisToolFreehand::beginPrimaryAction(event);
    if (mode() == KisTool::PAINT_MODE) {
        holdTimer_.start();
    } else {
        lifecycle_.interrupt(quickshape::Interruption::unsupported_node);
    }
}

void QuickShapeTool::continuePrimaryAction(KoPointerEvent* event) {
    if (replacementRunning_) return;

    KisToolFreehand::continuePrimaryAction(event);
    const auto sample = sampleFromEvent(event);
    const bool sampleAppended = lifecycle_.append(sample);
    KIS_SAFE_ASSERT_RECOVER_RETURN(sampleAppended);
    restartHoldTimer(sample);
}

void QuickShapeTool::endPrimaryAction(KoPointerEvent* event) {
    holdTimer_.stop();
    KisToolFreehand::endPrimaryAction(event);
    lifecycle_.finish();
    replacementRunning_ = false;
}

void QuickShapeTool::restartHoldTimer(const quickshape::Sample& sample) {
    const double dx = sample.position.x - holdAnchor_.x;
    const double dy = sample.position.y - holdAnchor_.y;
    if (std::hypot(dx, dy) > kMaximumEndpointDriftPixels) {
        holdAnchor_ = sample.position;
        holdTimer_.start();
    }
}

void QuickShapeTool::qualifyEndpointHold() {
    if (mode() != KisTool::PAINT_MODE || replacementRunning_ ||
        !lifecycle_.hold_qualifies(strokeClock_.nsecsElapsed() / 1000) ||
        !lifecycle_.begin_replay()) {
        return;
    }

    const quickshape::Sample endpoint = lifecycle_.captured().back();
    helper_->cancelRunningStroke();
    paintingInformationBuilder()->reset();
    replacementRunning_ = helper_->beginEndpointReplay(
        endpoint, image(), currentNode(), image().data());
    if (!replacementRunning_) {
        cancelCapture(quickshape::Interruption::replay_failed);
    }
}

void QuickShapeTool::cancelCapture(quickshape::Interruption reason) {
    holdTimer_.stop();
    helper_->cancelRunningStroke();
    paintingInformationBuilder()->reset();
    replacementRunning_ = false;
    lifecycle_.interrupt(reason);
    setMode(KisTool::HOVER_MODE);

    if (auto* canvas2 = dynamic_cast<KisCanvas2*>(canvas())) {
        canvas2->viewManager()->enableControls();
    }
}

void QuickShapeTool::requestStrokeCancellation() {
    if (mode() == KisTool::PAINT_MODE) {
        cancelCapture(quickshape::Interruption::escape);
    }
}

void QuickShapeTool::requestStrokeEnd() {
    if (mode() == KisTool::PAINT_MODE) {
        cancelCapture(quickshape::Interruption::node_changed);
    }
}

void QuickShapeTool::deactivate() {
    if (mode() == KisTool::PAINT_MODE) {
        cancelCapture(quickshape::Interruption::tool_deactivated);
    }
    KisToolFreehand::deactivate();
}

QuickShapeToolFactory::QuickShapeToolFactory()
    : KisToolPaintFactoryBase("KritaShape/QuickShapeTool") {
    setToolTip(i18n("QuickShape Tool"));
    setSection(ToolBoxSection::Shape);
    setIconName(koIconNameCStr("krita_tool_freehand"));
    setPriority(11);
    setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
}

KoToolBase* QuickShapeToolFactory::createTool(KoCanvasBase* canvas) {
    return new QuickShapeTool(canvas);
}
