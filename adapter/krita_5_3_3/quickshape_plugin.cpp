#include "quickshape_tool.hpp"

#include <KoToolRegistry.h>
#include <QDebug>
#include <kpluginfactory.h>

class QuickShapePlugin final : public QObject {
    Q_OBJECT

public:
    QuickShapePlugin(QObject* parent, const QVariantList&)
        : QObject(parent) {
        KoToolRegistry::instance()->add(new QuickShapeToolFactory());
        qInfo().noquote() << "QuickShape: registered KritaShape/QuickShapeTool";
    }
};

K_PLUGIN_FACTORY_WITH_JSON(QuickShapePluginFactory,
                           "quickshape_plugin.json",
                           registerPlugin<QuickShapePlugin>();)

#include "quickshape_plugin.moc"
