{
    "targets": [{
        "target_name": "bpxwdyn",
        "include_dirs": [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        "conditions": [
          [ "OS==\"zos\"", {
            "sources": [
               "addon.cc"
            ],
          }],
        ],

        "libraries": [],
        "dependencies": [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS", "__PTR32" ]
    }]
}
