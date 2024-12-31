const grpc = require("@grpc/grpc-js")
const message_proto = require("./proto")
const const_module = require("./const")
const {v4: uuidv4} = require("uuid")
const emailModule = require("./email")
const redis_module = require("./redis")

async function GetVarifyCode(call, callback) {  // call为请求，callback为回复（相当于request和response）  async声明了一个异步函数，在这里面才能使用下面的await
    console.log("email is ", call.request.email)
    try{
        let query_res = await redis_module.GetRedis(const_module.code_prefix+call.request.email);   // 如邮箱为123@163.com，则加上前缀拼接成code_123@163.com，其对应的值为uuid
        console.log("query_res is ", query_res)
        let uniqueId = query_res;
        if(query_res ==null){  // 找不到对应的值说明之前的验证码已过期或者还没发送过验证码，重新发送一下
            uniqueId = uuidv4();    // 得到验证码
            if (uniqueId.length > 4) {  // 只要验证码前四位
                uniqueId = uniqueId.substring(0, 4);
            } 
            let bres = await redis_module.SetRedisExpire(const_module.code_prefix+call.request.email, uniqueId,180) // 验证码过期时间为180秒

            let test = await redis_module.GetRedis(const_module.code_prefix+call.request.email); 
            console.log("键值对已创建，键：", const_module.code_prefix+call.request.email, " 值：", test)

            if(!bres){
                callback(null, { email:  call.request.email,
                    error:const_module.Errors.RedisErr
                });
                return;
            }
        }
        console.log("验证码还未过期，键：", const_module.code_prefix+call.request.email, " 值：", test)


        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'，请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: '2464861696@qq.com',
            to: call.request.email, //   邮件发送到客户端给的邮箱
            subject: '验证码',
            text: text_str,
        };

        let send_res = await emailModule.SendMail(mailOptions); //SendMail返回的promise，用await能等待发送完成后再执行下一行
        console.log("send res is ", send_res)

        if(!send_res){  // 邮件发送失败
            callback(null, { email:  call.request.email,
                error:const_module.Errors.RedisErr
            });
            return
        }

        callback(null, { email:  call.request.email,    // 向grpc客户端回传信息，null为默认参数，{}里为返回的json信息
            error:const_module.Errors.Success
        }); 


    }catch(error){  // 若SendMail出现异常，用try-catch能捕获reject中的异常结果
        console.log("catch error is ", error)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception // error返回异常信息
        }); 
    }

}

function main() {
    var server = new grpc.Server()  // 启动grpc Server
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {  // 服务器绑定端口，grpc.ServerCredentials.createInsecure()为安全校验，后面是个回调
        server.start()
        console.log('grpc server started')        
    })
}

main()