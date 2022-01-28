{
  "targets": [
    {
      "target_name": "basic",
      "include_dirs": [
          "<!(node -e \"require('nan')\")"                  
      ],
      "sources": ["c_src/basic_node.cc"]
    }
  ]
}
