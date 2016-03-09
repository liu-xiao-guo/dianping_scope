#ifndef API_CLIENT_H_
#define API_CLIENT_H_

#include <api/config.h>

#include <atomic>
#include <deque>
#include <map>
#include <string>
#include <core/net/http/request.h>
#include <core/net/uri.h>

#include <QJsonDocument>

namespace api {

/**
 * Provide a nice way to access the HTTP API.
 *
 * We don't want our scope's code to be mixed together with HTTP and JSON handling.
 */
class Client {
public:

    /**
     * Data structure for the quried data
     */
    struct Data {
        std::string name;
        std::string business_url;
        std::string s_photo_url;
        std::string photo_url;
        std::string rating_s_img_url;
        std::string address;
        std::string telephone;
        std::string summary;
    };

    /**
     * A list of weather information
     */
    typedef std::deque<Data> DataList;


    Client(Config::Ptr config);

    virtual ~Client() = default;

    /**
     * Get the query data
     */
    virtual DataList getData(const std::string &query);

    /**
     * Cancel any pending queries (this method can be called from a different thread)
     */
    virtual void cancel();

    virtual Config::Ptr config();

    void setLimit(int limit);
    void setCoordinate(QString longitude, QString latitude);

protected:
    void get(QString uri, QJsonDocument &root);

    /**
     * Progress callback that allows the query to cancel pending HTTP requests.
     */
    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

private:
    QString getQueryString(QString query);
    QString removeTestInfo(QString name);

    /**
     * Hang onto the configuration information
     */
    Config::Ptr config_;

    /**
     * Thread-safe cancelled flag
     */
    std::atomic<bool> cancelled_;

    int m_limit;
    QString m_longitude;
    QString m_latitude;
};

}

#endif // API_CLIENT_H_

