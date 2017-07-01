module.exports = [
    {
        "type": "heading",
        "defaultValue": "Planetary Clock"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "toggle",
                "messageKey": "ConfigInverted",
                "label": "Inverted mode",
                "defaultValue": false
            },
            {
                "type": "toggle",
                "messageKey": "ConfigShowBubbles",
                "label": "Show bubbles",
                "defaultValue": true
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
];
