import QtQuick 2.0
import QtDigitalAdvertising 1.1

Item{
MobileConfig {
    qdaApiKey: "B60E3B46-1318-BCB2-802B-77162E663AF6"
    networkId: "4147"
    siteId: "492524"
    appName: "BoltGenerator"
    bundleId: "scimulate.com"
    width: 320; height: 250
}

MobileAd {
    id: ad
    width: 320; height: 250
    playMode: QtDigitalAdvertising.AutoPlay
    displayMode: QtDigitalAdvertising.Loop
    pageId: "1539335"
    formatId: "107463"
}
}
