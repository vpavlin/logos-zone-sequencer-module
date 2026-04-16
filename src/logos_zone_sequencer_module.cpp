#include "logos_zone_sequencer_module.h"
#include "zone_sequencer.h"
#include "logos_api.h"
#include "logos_api_provider.h"
#include <QDebug>

LogosZoneSequencerModule::LogosZoneSequencerModule() {}

LogosZoneSequencerModule::~LogosZoneSequencerModule() {
    if (m_sequencerHandle) {
        zone_sequencer_destroy(m_sequencerHandle);
        m_sequencerHandle = nullptr;
        qInfo() << "ZoneSequencer: sequencer destroyed";
    }
}

void LogosZoneSequencerModule::initLogos(LogosAPI* api) {
    logosAPI = api;
    qInfo() << "ZoneSequencer: initLogos called";
    api->getProvider()->registerObject(name(), this);
    qInfo() << "ZoneSequencer: registered as remote object:" << name();
}

void LogosZoneSequencerModule::set_node_url(const QString& url) {
    m_nodeUrl = url;
    qInfo() << "ZoneSequencer: node_url =" << url;
    tryCreateSequencer();
}

void LogosZoneSequencerModule::set_signing_key(const QString& hex) {
    m_signingKey = hex;
    qInfo() << "ZoneSequencer: signing_key set, len=" << hex.length();
    tryCreateSequencer();
}

void LogosZoneSequencerModule::set_checkpoint_path(const QString& path) {
    m_checkpointPath = path;
    qInfo() << "ZoneSequencer: checkpoint_path =" << path;
    tryCreateSequencer();
}

void LogosZoneSequencerModule::set_channel_id(const QString& channelIdHex) {
    m_channelId = channelIdHex;
    qInfo() << "ZoneSequencer: channel_id =" << channelIdHex.left(16) + "...";
    tryCreateSequencer();
}

void LogosZoneSequencerModule::tryCreateSequencer() {
    if (m_sequencerHandle) return;
    if (m_nodeUrl.isEmpty() || m_signingKey.isEmpty() || m_channelId.isEmpty()) return;

    qInfo() << "ZoneSequencer: creating persistent sequencer...";
    m_sequencerHandle = zone_sequencer_create(
        m_nodeUrl.toUtf8().constData(),
        m_channelId.toUtf8().constData(),
        m_signingKey.toUtf8().constData(),
        m_checkpointPath.toUtf8().constData());

    if (m_sequencerHandle) {
        qInfo() << "ZoneSequencer: persistent sequencer created";
    } else {
        qWarning() << "ZoneSequencer: failed to create persistent sequencer";
    }
}

QString LogosZoneSequencerModule::get_channel_id() {
    if (!m_channelId.isEmpty()) return m_channelId;
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
    if (!m_sequencerHandle) {
        return QStringLiteral("Error: sequencer not initialized (call set_channel_id first)");
    }
    qInfo() << "ZoneSequencer: publishing via persistent sequencer";
    char* result = zone_sequencer_publish(m_sequencerHandle, data.toUtf8().constData());
    if (!result) {
        return QStringLiteral("Error: zone_sequencer_publish returned null");
    }
    QString txHash = QString::fromUtf8(result);
    zone_free_string(result);
    return txHash;
}

QString LogosZoneSequencerModule::query_channel(const QString& channelId, int limit) {
    char* result = zone_query_channel(
        m_nodeUrl.toUtf8().constData(),
        channelId.toUtf8().constData(),
        limit);
    if (!result) return QStringLiteral("[]");
    QString json = QString::fromUtf8(result);
    zone_free_string(result);
    return json;
}

QString LogosZoneSequencerModule::query_channel_paged(const QString& channelId,
                                                       const QString& cursorJson,
                                                       int limit) {
    const char* cursor = cursorJson.isEmpty() ? nullptr : cursorJson.toUtf8().constData();
    char* result = zone_query_channel_paged(
        m_nodeUrl.toUtf8().constData(),
        channelId.toUtf8().constData(),
        cursor,
        limit);
    if (!result) return QStringLiteral("{}");
    QString json = QString::fromUtf8(result);
    zone_free_string(result);
    return json;
}
