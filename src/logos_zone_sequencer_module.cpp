#include "logos_zone_sequencer_module.h"
#include "zone_sequencer.h"
#include "logos_api.h"
#include "logos_api_provider.h"
#include <QDebug>

LogosZoneSequencerModule::LogosZoneSequencerModule() {}
LogosZoneSequencerModule::~LogosZoneSequencerModule() {}

void LogosZoneSequencerModule::initLogos(LogosAPI* api) {
    logosAPI = api;
    qInfo() << "ZoneSequencer: initLogos called";
    api->getProvider()->registerObject(name(), this);
    qInfo() << "ZoneSequencer: registered as remote object:" << name();
}

void LogosZoneSequencerModule::set_node_url(const QString& url) {
    m_nodeUrl = url;
    qInfo() << "ZoneSequencer: node_url =" << url;
}

void LogosZoneSequencerModule::set_signing_key(const QString& hex) {
    m_signingKey = hex;
    qInfo() << "ZoneSequencer: signing_key set, len=" << hex.length();
}

void LogosZoneSequencerModule::set_checkpoint_path(const QString& path) {
    m_checkpointPath = path;
    qInfo() << "ZoneSequencer: checkpoint_path =" << path;
}

QString LogosZoneSequencerModule::get_channel_id() {
    if (m_signingKey.isEmpty()) {
        return QStringLiteral("Error: signing key not set");
    }
    char* result = zone_derive_channel_id(m_signingKey.toUtf8().constData());
    if (!result) {
        return QStringLiteral("Error: zone_derive_channel_id returned null");
    }
    QString channelId = QString::fromUtf8(result);
    zone_free_string(result);
    return channelId;
}

QString LogosZoneSequencerModule::publish(const QString& data) {
    if (m_signingKey.isEmpty()) {
        return QStringLiteral("Error: signing key not set");
    }
    qInfo() << "ZoneSequencer: publishing via" << m_nodeUrl << "checkpoint=" << m_checkpointPath;
    char* result = zone_publish(
        m_nodeUrl.toUtf8().constData(),
        m_signingKey.toUtf8().constData(),
        data.toUtf8().constData(),
        m_checkpointPath.toUtf8().constData());
    if (!result) {
        return QStringLiteral("Error: zone_publish returned null");
    }
    QString txHash = QString::fromUtf8(result);
    zone_free_string(result);
    return txHash;
}

QString LogosZoneSequencerModule::query_channel(const QString& channelId, int limit) {
    qInfo() << "ZoneSequencer: query_channel" << channelId.left(16) << "limit=" << limit;
    char* result = zone_query_channel(
        m_nodeUrl.toUtf8().constData(),
        channelId.toUtf8().constData(),
        limit);
    if (!result) return QStringLiteral("[]");
    QString json = QString::fromUtf8(result);
    zone_free_string(result);
    return json;
}
