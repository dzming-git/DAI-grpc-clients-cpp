# 微服务系统中的DAI-gRPC客户端子模块

本项目为微服务系统中其他使用Python语言开发的服务提供gRPC客户端代码。我们利用Protocol Buffers (protobuf) 定义了服务接口，并通过gRPC生成了相应的Python客户端和服务端代码。此外，我们在生成的代码基础上进一步封装了一层API，以简化服务间通信的开发工作。

该项目作为子模块使用，对于使用该子模块的项目，gRPC代码按照下述方式管理

## 项目结构

```
.
├── generated                 # 自动生成的gRPC代码目录
│   ├── protos                # 按模块分组的protobuf定义文件和生成的代码
│   │   ├── module1           # module1 的gRPC Python代码
│   │   │   ├── module1.grpc.pb.cc
│   │   │   ├── module1.grpc.pb.h
│   │   │   ├── module1.pb.cc
│   │   │   └── module1.pb.h
│   │   ├── module2           # module2 的gRPC Python代码
│   │   │   ├── module2.grpc.pb.cc
│   │   │   ├── module2.grpc.pb.h
│   │   │   ├── module2.pb.cc
│   │   │   └── module2.pb.h
```

## gRPC依赖版本

```
grpc 1.60.0
```
