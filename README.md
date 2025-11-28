# LLVM-Pass-For-Beginner
安装LLVM以及学习LLVM Pass

# 下载
```bash
git clone https://github.com/YLChenZ/LLVM-Pass-For-Beginner.git
```

# 构建Pass
### **<!--注意：下面都是在MyPasses目录下-->**
```bash
cd MyPasses
```

## 构建所有默认开启的Pass：
```bash
cmake -S . -B build
cmake --build build -j8
```

## 构建单个Pass（例如：只构建Pass2）
```bash
cmake -S . -B build -DBUILD_PASS1=OFF -DBUILD_PASS2=ON -DBUILD_PASS3=OFF
cmake --build build --target pass2 -j8
```
