#include <scope/preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>

namespace sc = unity::scopes;

using namespace std;
using namespace scope;

using namespace unity::scopes;
#include <QDebug>

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    // Support three different column layouts
    // Client can display Previews differently depending on the context
    // By creates two layouts (one with one column, one with two) and then
    // adding widgets to them differently, Unity can pick the layout the
    // scope developer thinks is best for the mode
    ColumnLayout layout1col(1), layout2col(2);

    // add columns and widgets (by id) to layouts.
    // The single column layout gets one column and all widets
    layout1col.add_column({"headerId", "artId", "infoId", "telId", "actionsId"});
    // The two column layout gets two columns.
    // The first column gets the art and header widgets (by id)
    layout2col.add_column({"artId", "headerId"});
    // The second column gets the info and actions widgets
    layout2col.add_column({"infoId", "telId", "actionsId"});

    // Push the layouts into the PreviewReplyProxy intance, thus making them
    // available for use in Preview diplay
    reply->register_layout({layout1col, layout2col});

    //Create some widgets
    // header type first. note 'headerId' used in layouts
    // second field ('header) is a standard preview widget type
    PreviewWidget w_header("headerId", "header");
    // This maps the title field of the header widget (first param)  to the
    // title field in the result to be displayed in this preview, thus providing
    // the result-specific data to the preview for display
    w_header.add_attribute_mapping("title", "title");

    // Standard subtitle field here gets our 'artist' key value
    // w_header.add_attribute_mapping("subtitle", "artist");

    PreviewWidget w_art("artId", "image");
    w_art.add_attribute_mapping("source", "art");

    PreviewWidget w_info("infoId", "text");
    w_info.add_attribute_mapping("text", "address");

    PreviewWidget w_tel("telId", "text");
    w_tel.add_attribute_mapping("text", "telephone");

    Result result = PreviewQueryBase::result();
    QString urlString(result["uri"].get_string().c_str());
    // qDebug() << "[Details] GET " << urlString;
   // QUrl url = QUrl(urlString);

    // Create an Open button and provide the URI to open for this preview result
    PreviewWidget w_actions("actionsId", "actions");
    VariantBuilder builder;
    builder.add_tuple({
            {"id", Variant("open")},
            {"label", Variant("Open")},
            {"uri", Variant(urlString.toStdString())} // uri set, this action will be handled by the Dash
        });
    w_actions.add_attribute_value("actions", builder.end());

    // Bundle out widgets as required into a PreviewWidgetList
    PreviewWidgetList widgets({w_header, w_art, w_info, w_tel, w_actions});
    // And push them to the PreviewReplyProxy as needed for use in the preview
    reply->push(widgets);
}
