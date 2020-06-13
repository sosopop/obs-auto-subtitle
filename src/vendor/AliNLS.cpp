//
// Created by Yibai Zhang on 2020/6/10.
//
#include <QDebug>
#include <QCryptographicHash>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrlQuery>
#include <QUuid>

#include "AliNLS.h"

using namespace std::placeholders;

AliNLS::AliNLS(const QString &appKey, const QString &token, QObject *parent)
        : ASRBase(parent), appKey(appKey), token(token){
    connect(&ws, &QWebSocket::connected, this, &AliNLS::onConnected);
    connect(&ws, &QWebSocket::disconnected, this, &AliNLS::onDisconnected);
    connect(&ws, &QWebSocket::textMessageReceived, this, &AliNLS::onTextMessageReceived);
    connect(this, &ASRBase::sendAudioMessage, this, &AliNLS::onSendAudioMessage);
    connect(this, &AliNLS::haveResult, this, &AliNLS::onResult);
    connect(&ws, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    task_id = QUuid::createUuid().toRfc4122().toHex();
    _header["task_id"] = task_id;
    _header["namespace"] = "SpeechTranscriber";
    running = false;
}

void AliNLS::onStart(){
    QNetworkRequest request;
    QUrl url(ALINLS_URL);
    qDebug()<< url.toString();
    QUrlQuery query;

    url.setQuery(query);
    request.setUrl(url);
    request.setRawHeader(ALINLS_TOKEN_HEADER, token.toLocal8Bit());
    ws.open(request);
}

void AliNLS::onError(QAbstractSocket::SocketError error) {
    qDebug()<< ws.errorString();
    qDebug() << error;
}

void AliNLS::onConnected() {
    running = true;
    _header["name"] = "StartTranscription";
    _payload["sample_rate"] = 16000;
    _payload["format"] = "pcm";
    _payload["enable_inverse_text_normalization"] = true;
    _payload["enable_intermediate_result"] = true;
    _payload["enable_punctuation_prediction"] = false;
    _payload["max_sentence_silence"] = 400;

    ws.sendTextMessage(serializeReq());
    qDebug() << "WebSocket connected";
}

void AliNLS::onDisconnected() {
    running = false;
    qDebug() << "WebSocket disconnected";

}


void AliNLS::onSendAudioMessage(const void *data, unsigned long size){
    if(! running){
        return;
    }
    ws.sendBinaryMessage(QByteArray::fromRawData((const char*)data, size));
}


void AliNLS::onTextMessageReceived(const QString message) {
    QJsonDocument doc(QJsonDocument::fromJson(message.toUtf8()));

    bool ok = false;
    QString output;
    int type;
    if(doc["header"]["name"].toString() == "TranscriptionResultChanged") {
        output = doc["payload"]["result"].toString();
        type = ResultType_Middle;
        ok = true;
    } else if (doc["header"]["name"].toString() == "SentenceEnd") {
        output = doc["payload"]["result"].toString();
        type = ResultType_End;
        ok = true;
    } else {
        qDebug() << message;
    }
    if(!ok){
        return;
    }
    emit haveResult(output, type);
}

void AliNLS::onResult(QString message, int type) {
    auto callback = getCallback();
    if(callback)
        callback(message, type);
}

void AliNLS::onStop() {
    _header["name"] = "StopTranscription";
    ws.sendTextMessage(serializeReq());
    ws.close();
}

QString AliNLS::getAppKey() {
    return appKey;
}

QString AliNLS::getToken() {
    return token;
}

QString AliNLS::serializeReq() {
    QJsonDocument doc;
    QJsonObject root;
    QJsonObject header;
    _header["message_id"] = QUuid::createUuid().toRfc4122().toHex();
    _header["appkey"] = appKey;
    for (auto i = _header.begin(); i != _header.end(); ++i){
        header.insert(i.key(), QJsonValue(i.value()));
    }
    root.insert("header", header);
    QJsonObject payload;
    for (auto i = _payload.begin(); i != _payload.end(); ++i){
        payload.insert(i.key(), i.value());
    }
    root.insert("payload", payload);
    doc.setObject(root);
    _payload.clear();
    return doc.toJson();
}


AliNLS::~AliNLS() {
    stop();
}
