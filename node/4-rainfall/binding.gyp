{
  "targets": [
    {
      "target_name": "rainfall",
      "sources": ["c_src/rainfall_node.cc", "c_src/rainfall.cc"],
      "cflags": ["-Wall", "-std=c++11"]
    }
  ]
}
