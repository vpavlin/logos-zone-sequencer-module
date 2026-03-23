#pragma once
#include "i_logos_zone_sequencer_module.h"
#include <QObject>
#include <QString>

class LogosAPI;

class LogosZoneSequencerModule : public QObject, public ILogosZoneSequencerModule {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ILogosZoneSequencerModule_iid FILE LOGOS_ZONE_SEQUENCER_MODULE_METADATA_FILE)
    Q_INTERFACES(PluginInterface ILogosZoneSequencerModule)
public:
    LogosZoneSequencerModule();
    ~LogosZoneSequencerModule() override;
    QString name() const override { return "liblogos_zone_sequencer_module"; }
    QString version() const override { return "0.1.0"; }
    Q_INVOKABLE void initLogos(LogosAPI* api) override;
    Q_INVOKABLE void set_node_url(const QString& url) override;
    Q_INVOKABLE void set_signing_key(const QString& hex) override;
    Q_INVOKABLE void set_checkpoint_path(const QString& path) override;
    Q_INVOKABLE QString publish(const QString& data) override;
signals:
    void eventResponse(const QString& eventName, const QVariantList& data);
private:
    QString m_nodeUrl = QStringLiteral("http://localhost:8080");
    QString m_signingKey;
    QString m_checkpointPath;
};
