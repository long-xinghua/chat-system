const nodemailer = require('nodemailer');   //发送邮件的库
const config_module = require("./config")   //包含当前路径下的config.js，就能使用里面解析出来的配置信息

/**
 * 创建发送邮件的代理
 */
let transport = nodemailer.createTransport({
    host: 'smtp.qq.com',    
    port: 465,  // QQ邮箱还可以用587端口
    secure: true,   // 有安全验证
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass // 邮箱授权码或者密码
    }
});

/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_){
    return new Promise(function(resolve, reject){   // 通过一层包装把异步的sendMail包装成同步的
        transport.sendMail(mailOptions_, function(error, info){ // transport中的sendmail函数和C++中异步函数一样会马上返回，但并不一定成功，要通过回调函数function(error, info)来通知
            if (error) {
                console.log(error);
                reject(error);  // 异常情况调用reject
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)  // 正常情况调用resolve
            }
        });
    })

}

module.exports.SendMail = SendMail  // 导出SendMail函数给其他文件用，用于发送邮件