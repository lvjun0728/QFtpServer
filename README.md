# QFtpServer


QFtpServer 
基于QT5.12.7测试通过
支持TLS加密
支持多用户,一个用户一个线程.
支持定义数据端口范围.


//关于秘钥生成
1.下载OpenSSL 软件 http://slproweb.com/products/Win32OpenSSL.html

首先得安装OpenSSL软件包openssl，安装了这个软件包之后，我们可以做这些事情： 
  o  Creation of RSA, DH and DSA Key Parameters # 创建密钥 key 
  o  Creation of X.509 Certificates, CSRs and CRLs # 创建证书 
  o  Calculation of Message Digests                # 
  o  Encryption and Decryption with Ciphers # 加密、解密 
  o  SSL/TLS Client and Server Tests        # SSL 服务器端/客户端测试 
  o  Handling of S/MIME signed or encrypted Mail  # 处理签名或加密了的邮件 

1、生成RSA密钥的方法 
//可用的秘钥加密算法 des|-des3|-idea|-aes128|-aes192|-aes256
openssl genrsa -des3 -passout pass:"123456" -out privkey.pem 2048 //生成一个带密码的秘钥文件,密码算法des3
openssl genrsa -des -passout pass:"123456" -out privkey.pem 2048 //生成一个带密码的秘钥文件,密码算法des
openssl genrsa -aes128 -passout pass:"123456" -out privkey.pem 2048 //生成一个带密码的秘钥文件,密码算法idea
openssl genrsa -out privkey.pem 2048  		//生成一个无密码的秘钥文件
2、生成一个证书请求 
openssl req -new -key privkey.pem -out cert.csr  //无密码
openssl req -new -key privkey.pem -passin pass:"123456" -out cert.csr //有密码

这个命令将会生成一个证书请求，当然，用到了前面生成的密钥privkey.pem文件 
这里将生成一个新的文件cert.csr，即一个证书请求文件，你可以拿着这个文件去数字证书颁发机构（即CA）申请一个数字证书。CA会给你一个新的文件cacert.pem，那才是你的数字证书。 

如果是自己做测试，那么证书的申请机构和颁发机构都是自己。就可以用下面这个命令来生成证书： 
openssl req -new -x509 -key privkey.pem -out cacert.pem -days 1095 //无密码
openssl req -new -x509 -key privkey.pem -passin pass:"123456" -out cacert.pem -days 1095 //有密码

这个命令将用上面生成的密钥 privkey.pem 生成一个数字证书cacert.pem 

3、使用数字证书和密钥 
有了privkey.pem和cacert.pem文件后就可以在自己的程序中使用了，比如做一个加密通讯的服务器



