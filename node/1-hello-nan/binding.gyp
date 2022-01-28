{
  "targets": [
    {
      "target_name": "hello",
      "include_dirs": [
          "<!(node -e \"require('nan')\")"                  
      ],
      "sources": ["c_src/hello_node.cc"]
    }
  ]
}
