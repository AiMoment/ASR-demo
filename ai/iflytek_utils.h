#ifndef IFLYTEK_UTILS_H
#define IFLYTEK_UTILS_H

#include <string>
#include <sstream>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

/**
 * @brief hmac_sha256算法，利用key对data进行加密认证
 * @param data 待hmac_sha256的数据
 * @param key hmac_sha256的密钥
 * @return 256位加密字符串
 */
std::string get_hmac_sha256(const std::string &data, const std::string &key);

/**
 * @brief base64编码算法
 * @param data 待编码数据
 * @return data被base64编码后的字符串
 */
std::string get_base64_encode(const std::string &data);

/**
 * @brief base64解码算法
 * @param data 待解码数据
 * @return data被base64解码后的字符串
 */
std::string get_base64_decode(const std::string &data);

/**
 * @brief url编码算法，转义url中的特殊字符和中文字符等
 * @param url 待编码的url
 * @return 被编码转义后的url
 */
std::string getm_authorizeUrl_encode(const std::string &url);

/**
 * @brief 延迟函数，Windows和Linux下的延迟函数各异
 * @param t 延迟秒数
 */
void delay(double t);

/**
 * @brief md5算法，对data进行md5认证处理
 * @param data 待md5的数据
 * @return 128位字符的散列值，讯飞要求md5后为小写十六进制数据
 */
std::string get_md5(const std::string &data);

/**
 * @brief hmac_sha1算法，利用key对data进行加密认证处理
 * @param data 待hmac_sha1的数据
 * @param key hmac_sha1的密钥
 * @retrun 160位加密字符串
 */
std::string get_hmac_sha1(const std::string &data, const std::string &key);

#endif // IFLYTEK_UTILS_H
