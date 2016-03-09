#include <boost/algorithm/string/trim.hpp>

#include <scope/localization.h>
#include <scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>

#include <unity/scopes/SearchMetadata.h>
#include <QDebug>

#include <iomanip>
#include <sstream>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace api;
using namespace scope;
using namespace unity::scopes;

const QString DEFAULT_LATITUDE = "39.9698";
const QString DEFAULT_LONGITUDE = "116.446";

// Create a JSON string to be used tro create a category renderer - uses grid layout
const std::string CR_GRID = R"(
{
        "schema-version" : 1,
        "template" : {
            "category-layout" : "vertical-journal",
            "card-layout": "horizontal",
            "card-size": "small",
            "collapsed-rows": 0
        },
        "components" : {
            "title":"title",
            "subtitle":"subtitle",
            "summary":"summary",
            "art":{
                "field": "art2",
                "aspect-ratio": 1
            }
        }
})";

// Create a JSON string to be used tro create a category renderer - uses carousel layout
const std::string CR_CAROUSEL = R"(
{
     "schema-version" : 1,
     "template" : {
        "category-layout" : "carousel",
        "card-size": "small",
        "overlay" : true
      },
      "components" : {
        "title":"title",
        "art": {
        "field": "art",
        "aspect-ratio": 1.6,
        "fill-mode": "fit"
        }
      }
})";

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             Config::Ptr config) :
    sc::SearchQueryBase(query, metadata), client_(config) {
}

void Query::cancelled() {
    client_.cancel();
}


void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        initScope();

        // Get the current location of the search
        auto metadata = search_metadata();
        if ( metadata.has_location() ) {
            qDebug() << "Location is supported!";
            auto location = metadata.location();

            if ( location.has_altitude()) {
                cerr << "altitude: " << location.altitude() << endl;
                cerr << "longitude: " << location.longitude() << endl;
                cerr << "latitude: " << location.latitude() << endl;
                auto latitude = std::to_string(location.latitude());
                auto longitude = std::to_string(location.longitude());
                m_longitude = QString::fromStdString(longitude);
                m_latitude = QString::fromStdString(latitude);
            }

            if ( m_longitude.isEmpty() ) {
                m_longitude = DEFAULT_LONGITUDE;
            }
            if ( m_latitude.isEmpty() ) {
                m_latitude = DEFAULT_LATITUDE;
            }

            qDebug() << "m_longitude1: " << m_longitude;
            qDebug() << "m_latitude1: " << m_latitude;
        } else {
            qDebug() << "Location is not supported!";
            m_longitude = DEFAULT_LONGITUDE;
            m_latitude = DEFAULT_LATITUDE;
        }

        client_.setCoordinate(m_longitude, m_latitude);

        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());
        QString queryString = QString::fromStdString(query.query_string());

        // Trim the query string of whitespace
        string query_string = alg::trim_copy(query.query_string());

        Client::DataList datalist;
        if (query_string.empty()) {
            queryString = QString("美食");
            datalist = client_.getData(queryString.toStdString());
        } else {
            // otherwise, get the forecast for the search string
            datalist = client_.getData(query_string);
        }

        CategoryRenderer rdrGrid(CR_GRID);
        CategoryRenderer rdrCarousel(CR_CAROUSEL);

        QString title = queryString;

        // Register two categories
        auto carousel = reply->register_category("dianpingcarousel", title.toStdString(), "", rdrCarousel);
        auto grid = reply->register_category("dianpinggrid", "", "", rdrGrid);

        bool isgrid = false;

        // For each of the entry in the datalist
        for (const Client::Data &data : datalist) {

            // for each result
            const std::shared_ptr<const Category> * category;

            if ( isgrid ) {
                category = &grid;
                isgrid = false;
            } else {
                isgrid = true;
                category = &carousel;
            }

            //create our categorized result using our pointer, which is either to out
            //grid or our carousel Category
            CategorisedResult catres((*category));

            // We must have a URI
            catres.set_uri(data.business_url);
            catres.set_dnd_uri(data.business_url);
            catres.set_title(data.name);

            catres["subtitle"] = data.address;
            catres["summary"] = data.summary;
            catres["fulldesc"] = data.summary;
            catres["art2"] = data.s_photo_url;

            catres.set_art(data.photo_url);

            catres["address"] = Variant(data.address);
            catres["telephone"] = Variant(data.telephone);

            // Push the result
            if (!reply->push(catres)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
        }
    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}

// The followoing function is used to retrieve the settings for the scope
void Query::initScope()
{
    unity::scopes::VariantMap config = settings();  // The settings method is provided by the base class
//    if (config.empty())
//        qDebug() << "CONFIG EMPTY!";

    int limit = config["limit"].get_double();
    cerr << "limit: " << limit << endl;

    client_.setLimit(limit);
}

