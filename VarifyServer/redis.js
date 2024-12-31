// 用于给redis发送带过期时间的验证码
const config_module = require("./config")
const Redis = require("ioredis")

// 创建redis客户端实例
const RedisCli = new Redis({
    host: config_module.redis_host, // 地址是字符串类型
    port: config_module.redis_port, //  port是数字
    password: config_module.redis_passwd  // 密码是字符串类型，password名字不能更改（如我之前写的passwd），这个名字是Redis类中定义好的
});

/**
 * 监听错误信息
 */
RedisCli.on("error", function (err) {   //on函数中有连接操作，如果连接失败就会调用里面的回调函数
    console.log("RedisCli connect error ："+err);
    RedisCli.quit();
  });

/**
 * 根据key获取value
 * @param {*} key 描述函数参数类型，*表示key可以是任何类型
 * @returns 
 */
async function GetRedis(key) {  // 要用await必须要给函数声明async

    try{
        const result = await RedisCli.get(key)  // 就相当于C++中用redisCommand给redis服务器发送命令了。await，类似于C++中future，等待RedisCli.get(key)完成后再执行下一步
        if(result === null){
          console.log('result:','<'+result+'>', 'This key cannot be find...')
          return null   // 指返回空
        }
        console.log('Result:','<'+result+'>','Get key success!...');
        return result
    }catch(error){
        console.log('GetRedis error is', error);
        return null
    }

  }

  /**
 * 根据key查询redis中是否存在key
 * @param {*} key 描述函数参数类型，*表示key可以是任何类型
 * @returns     描述返回值类型和含义，在这里没有指定
 */
async function QueryRedis(key) {
    try{
        const result = await RedisCli.exists(key)
        //  判断该值是否为空 如果为空返回null
        if (result === 0) {
          console.log('result:<','<'+result+'>','This key is null...');
          return null
        }
        console.log('Result:','<'+result+'>','With this value!...');
        return result
    }catch(error){
        console.log('QueryRedis error is', error);
        return null
    }

  }

  /**
 * 设置key和value，并过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} exptime 
 * @returns 
 */
async function SetRedisExpire(key,value, exptime){
    try{
        // 设置键和值
        await RedisCli.set(key,value)
        // 设置过期时间（以秒为单位）
        await RedisCli.expire(key, exptime);    // 在redis中对key设置过期时间后，到达指定时间它将被自动删除
        return true;
    }catch(error){
        console.log('SetRedisExpire error is', error);
        return false;
    }
}

function Quit(){
    RedisCli.quit();
}

module.exports = {GetRedis, QueryRedis, Quit, SetRedisExpire,}