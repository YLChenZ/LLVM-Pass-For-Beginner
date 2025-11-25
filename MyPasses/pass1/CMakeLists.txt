# 如果上层没导出某些东西，再次确保能找到 LLVM
if(NOT TARGET LLVM::Core OR NOT TARGET LLVM::Support)
  find_package(LLVM REQUIRED CONFIG)
  list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}")
  include(AddLLVM)
endif()

# 指定你需要的 LLVM 组件（根据实际需要增删）
# 常见的有: Core, Support, IRReader, Analysis, TransformUtils, Passes ...
set(REQ_LLVM_COMPONENTS Core Support)

# 把组件映射到库名 -> 得到变量 LLVM_LIBS
llvm_map_components_to_libnames(LLVM_LIBS ${REQ_LLVM_COMPONENTS})

add_library(pass1 MODULE pass1.cpp)

# 链接得到的库名（而不是 LLVM::Core 这类 target）
target_link_libraries(pass1 PRIVATE ${LLVM_LIBS})

target_compile_features(pass1 PRIVATE cxx_std_17)

set_target_properties(pass1 PROPERTIES
  PREFIX ""                                    # 去掉 lib 前缀，生成 pass1.so
  OUTPUT_NAME "pass1"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)