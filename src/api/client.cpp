#include <api/client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <QVariantMap>

#include <QString>
#include <QCryptographicHash>
#include <QDebug>
#include <QTextCodec>
#include <QUrl>

#include <iostream>

namespace http = core::net::http;
namespace net = core::net;

using namespace api;
using namespace std;
using namespace std;

const QString appkey = "3562917596";
const QString secret = "091bf584e9d24edbbf48971d65307be3";
const QString BASE_URI = "http://api.dianping.com/v1/business/find_businesses?";

Client::Client(Config::Ptr config) :
    config_(config), cancelled_(false) {
}

void Client::get( QString uri, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    configuration.uri = uri.toStdString();

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Client::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());

        // qDebug() << "response: " << response.body.c_str();
    } catch (net::Error &) {
    }
}

Client::DataList Client::getData(const string& query) {
    QJsonDocument root;

    QString temp = QString::fromStdString(query);
    QByteArray bytearray = query.c_str();
    QString query_string = QString::fromUtf8(bytearray.data(), bytearray.size());

    qDebug() << "query_string: " << query_string;

    QString uri = getQueryString(query_string);
    qDebug() << "uri: "  << uri;
    get(uri, root);

    DataList result;

    QVariantMap variant = root.toVariant().toMap();

    // Iterate through the weather data
    for (const QVariant &i : variant["businesses"].toList()) {
        QVariantMap item = i.toMap();

        QString name = removeTestInfo(item["name"].toString());
        qDebug() << "name: " << name;

        QString business_url = item["business_url"].toString();
        qDebug() << "business_url: " << business_url;

        QString s_photo_url = item["s_photo_url"].toString();
        qDebug() << "s_photo_url: " << s_photo_url;

        QString photo_url = item["photo_url"].toString();
        qDebug() << "photo_url: " << photo_url;

        QString rating_s_img_url = item["rating_s_img_url"].toString();
        qDebug() << "rating_s_img_url: " << rating_s_img_url;

        QString address = item["address"].toString();
        qDebug() << "address: " << address;

        QString telephone = item["telephone"].toString();
        qDebug() << "telephone: " << telephone;

        QVariantList deals = item["deals"].toList();

        QString summary;
        if ( deals.count() > 0 ) {
            QVariantMap temp = deals.first().toMap();
            summary = temp["description"].toString();
        }

        qDebug() << "summary: " << summary;

        // Add a result to the weather list
        result.emplace_back(
            Data { name.toStdString(), business_url.toStdString(), s_photo_url.toStdString(),
                   photo_url.toStdString(), rating_s_img_url.toStdString(),
                   address.toStdString(), telephone.toStdString(), summary.toStdString() });
    }

    return result;
}

http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Config::Ptr Client::config() {
    return config_;
}

void Client::setLimit(int limit)
{
    m_limit = limit;
}

void Client::setCoordinate(QString longitude, QString latitude)
{
    m_longitude = longitude;
    m_latitude = latitude;
}

QString Client::getQueryString(QString query) {
    QMap<QString, QString> map;

    map["category"] = query;
    // map["city"] = query;
    map["sort"] = "2";
    map["limit"] = QString::number(m_limit);
    map["platform"] = "2";

    map["latitude"] = m_latitude;
    map["longitude"] = m_longitude;

    QCryptographicHash generator(QCryptographicHash::Sha1);

    QString temp;
    temp.append(appkey);
    QMapIterator<QString, QString> i(map);
    while (i.hasNext()) {
        i.next();
        qDebug() << i.key() << ": " << i.value();
        temp.append(i.key()).append(i.value());
    }

    temp.append(secret);

    qDebug() << temp;

    qDebug() << "UTF-8: " << temp.toUtf8();

    generator.addData(temp.toUtf8());
    QString sign = generator.result().toHex().toUpper();

//    qDebug() << sign;

    QString url;
    url.append(BASE_URI);
    url.append("appkey=");
    url.append(appkey);

    url.append("&");
    url.append("sign=");
    url.append(sign);

    i.toFront();
    while (i.hasNext()) {
        i.next();
        qDebug() << i.key() << ": " << i.value();
        url.append("&").append(i.key()).append("=").append(i.value());
    }

    qDebug() << "Final url: " << url;

    QUrl url1(url);

    qDebug() << url1.url();
    QByteArray bytearray = url1.toEncoded();
    QString string = QString::fromUtf8(bytearray.data(), bytearray.size());
    qDebug() << "url new: " << string;

    return string;
}

// The following method is used to remove the
// "这是一条测试商户数据，仅用于测试开发，开发完成后请申请正式数据..." string
const QString TEST_STRING = "(这是一条测试商户数据，仅用于测试开发，开发完成后请申请正式数据...)";
QString Client::removeTestInfo(QString name)
{
    if ( name.contains(TEST_STRING) ) {
        int index = name.indexOf(TEST_STRING);
        QString newName = name.left(index);
        // qDebug() << "newName: " << newName;
        return newName;
    } else {
        qDebug() << "it does not contain the string";
        return name;
    }
}

