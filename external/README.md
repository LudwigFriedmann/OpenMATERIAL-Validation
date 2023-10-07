Third party software
====================

This directory contains the following third party software:

| Filepath              | Origin                                           | Description          | Files                                             | License                                                                     |
|:----------------------|:-------------------------------------------------|:---------------------|:--------------------------------------------------|:----------------------------------------------------------------------------|
| [argparse](argparse/) | [argparse](https://github.com/cofyc/argparse)    | Command line parsing | `argparse.c`, `argparse.h`                        | [MIT](https://opensource.org/licenses/MIT)                                  |                                 
| [doctest](doctest/)   | [doctest](https://github.com/doctest/doctest/)   | Unit testing         | `doctest.h`                                       | [MIT](https://opensource.org/licenses/MIT)                                  |
| [tinygltf](tinygltf)  | [tinygltf](https://github.com/syoyo/tinygltf)    | glTF loading         | `stb_image.h`, `stb_image_write.h`, `tiny_gltf.h` | [MIT](https://opensource.org/licenses/MIT)                                  |
| [json](json/)         | [json](https://github.com/nlohmann/json)         | Json loading         | `json.hpp`                                        | [MIT](https://opensource.org/licenses/MIT)                                  |
| [sort_r](sort_r/)     | [sort_r](https://github.com/noporpoise/sort_r)   | Portable `qsort_r`   | `sort_r.h`                                        | [Public Domain](https://github.com/noporpoise/sort_r/blob/master/README.md) |
| [CImg](CImg/)         | [CImg](https://cimg.eu/)                         | Texture reading      | `CImg.h`                                          | [CeCILL](http://www.cecill.info/licences/Licence_CeCILL_V2-en.html)         |


The software is accompanied by two interface implementations, `doctest.cpp` and `tiny_gltf.cpp`.

> **_NOTE:_** Besides the third-party dependencies mentioned above, the pathtracer software requires [Intel Embree v4.1.0](https://github.com/embree/embree/releases/tag/v4.1.0) as prerequisite, which is licensed under [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) license.