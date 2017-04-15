{
  "targets": [
    {
      "target_name": "localgroups",
      "sources": [
        "src/argsresolv.cc",
        "src/lgrperrs.cc",
        "src/lgapi-utils.cc",
        "src/lgrplist.cc",
        "src/lgrpenum.cc",
        "src/enumwrap.cc",
        "src/lgmemblist.cc",
        "src/lgmembers.cc",
        "src/membwrap.cc",
        "src/lgrpinfo.cc",
        "src/infowrap.cc",
        "src/module.cc"
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'AdditionalOptions': [ '/EHsc'] 
        }
      }
    }
  ]
}
