/*
    using openssl to encrypt/descript
*/

#pragma once
#include <string>
#include <vector>


class TGMTcrypto
{	
public:
	static std::string EncodeBase64(std::string value);
	static std::string DecodeBase64(std::string encoded_string);
	static int public_encrypt(std::string data, std::string key, std::string& encrypted);
	static int private_decrypt(std::string enc_data, std::string key, std::string& decrypted);
	static int private_encrypt(std::string data, std::string key, std::string& encrypted);
	static int public_decrypt(std::string enc_data, std::string key, std::string& decrypted);
	static std::string LoadPublicKey(std::string file);
	static std::string LoadPrivateKey(std::string file);

    //TGMTcrypto();
    //~TGMTcrypto();

    static std::string EncryptStringAES(std::string value, std::string password);
    static std::string DecryptStringAES(std::string value, std::string password);

	static bool EncryptFileAES(std::string infile, std::string outfile, std::string password);
	static bool DecryptFileAES(std::string infile, std::string outfile, std::string password);

	static std::string MD5_from_file(const std::string& path);

	static std::string GenerateHourlyOTP(std::string key);
};

