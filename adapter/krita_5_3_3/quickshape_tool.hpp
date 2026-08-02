#pragma once

#include <KisToolPaintFactoryBase.h>
#include <QElapsedTimer>
#include <QTimer>
#include <kis_tool_freehand.h>

#include "quickshape/host_contract.hpp"

class KoCanvasBase;

class QuickShapeTool final : public KisToolFreehand {
    Q_OBJECT

public:
    explicit QuickShapeTool(KoCanvasBase* canvas);
    ~QuickShapeTool() override = default;

    void deactivate() override;
    void requestStrokeCancellation() override;
    void requestStrokeEnd() override;

protected:
    void beginPrimaryAction(KoPointerEvent* event) override;
    void continuePrimaryAction(KoPointerEvent* event) override;
    void endPrimaryAction(KoPointerEvent* event) override;

private:
    class FreehandHelper;

    quickshape::Sample sampleFromEvent(const KoPointerEvent* event);
    void restartHoldTimer(const quickshape::Sample& sample);
    void qualifyEndpointHold();
    void cancelCapture(quickshape::Interruption reason);

    FreehandHelper* helper_{nullptr};
    quickshape::StrokeLifecycle lifecycle_;
    QElapsedTimer strokeClock_;
    QTimer holdTimer_;
    quickshape::Point holdAnchor_{};
    bool replacementRunning_{false};
};

class QuickShapeToolFactory final : public KisToolPaintFactoryBase {
public:
    QuickShapeToolFactory();
    KoToolBase* createTool(KoCanvasBase* canvas) override;
};
