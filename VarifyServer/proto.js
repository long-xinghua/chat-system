const path = require("path")    // 相当于C++中的include
const grpc = require('@grpc/grpc-js')
const protoloader = require('@grpc/proto-loader')   // 用于加载.proto文件，并转换为适用于JavaScript 的对象，供gRPC使用

const PROTO_PATH = path.join(__dirname, 'message.proto')
const packageDefiniton = protoloader.loadSync(PROTO_PATH, {keepCase:true, longs:String, enums:String, defaults:true, oneofs:true})

const protoDescriptor = grpc.loadPackageDefinition(packageDefiniton)    // grpc加载这个对象并进行解析

const message_proto =  protoDescriptor.message

module.exports = message_proto  // 将message_proto导出，其他文件只要引入本文件就能引用message_proto


