{
  "targets": [
    {
      "target_name": "localgroups",
      "sources": [
        "src/module.cc"
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}
