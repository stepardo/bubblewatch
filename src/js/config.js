module.exports = [
    {
        "type": "heading",
        "defaultValue": "Bubble Watch"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "toggle",
                "messageKey": "ConfigShowSeconds",
                "label": "Show seconds",
                "defaultValue": true
            },
            {
                "type": "toggle",
                "messageKey": "ConfigShowBubbles",
                "label": "Show values in bubbles",
                "defaultValue": true
            },
            {
                "type": "toggle",
                "messageKey": "ConfigVibrateOnBluetoothDisconnect",
                "label": "Vibrate on Bluetooth disconnect",
                "defaultValue": false
            },
            {
                "type": "color",
                "messageKey": "ConfigForegroundColor",
                "label": "Foreground Color",
                "defaultValue": "FFFFFF",
                "allowGray": false
            },
            {
                "type": "color",
                "messageKey": "ConfigBackgroundColor",
                "label": "Background Color",
                "defaultValue": "000000",
                "allowGray": false
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
];
