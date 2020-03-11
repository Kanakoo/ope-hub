//#include "myaes.h"
//using namespace std;
//void AES::initKV()
//{
//    memset( key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
//    memset( iv, 0x00, CryptoPP::AES::BLOCKSIZE );

//    // 或者也能够
//    /*
//    char tmpK[] = "1234567890123456";
//    char tmpIV[] = "1234567890123456";
//    for (int j = 0; j < CryptoPP::AES::DEFAULT_KEYLENGTH; ++j)
//    {
//        key[j] = tmpK[j];
//    }

//    for (int i = 0; i < CryptoPP::AES::BLOCKSIZE; ++i)
//    {
//        iv[i] = tmpIV[i];
//    }
//    */
//}
//string AES::encrypt(string plainText)
//{
//    string cipherText;

//    //
//    CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
//    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv );
//    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( cipherText ));
//    stfEncryptor.Put( reinterpret_cast<const unsigned char*>( plainText.c_str() ), plainText.length() + 1 );
//    stfEncryptor.MessageEnd();

//    string cipherTextHex;
//    for( int i = 0; i < cipherText.size(); i++ )
//    {
//        char ch[3] = {0};
//        sprintf(ch, "%02x",  static_cast<CryptoPP::byte>(cipherText[i]));
//        cipherTextHex += ch;
//    }

//    return cipherTextHex;
//}
//void AES::writeCipher(string output)
//{
//    ofstream out("/tmp/cipher.data");
//    out.write(output.c_str(), output.length());
//    out.close();

//    cout<<"writeCipher finish "<<endl<<endl;
//}
//string AES::decrypt(string cipherTextHex)
//{
//    string cipherText;
//    string decryptedText;

//    int i = 0;
//    while(true)
//    {
//        char c;
//        int x;
//        stringstream ss;
//        ss<<hex<<cipherTextHex.substr(i, 2).c_str();
//        ss>>x;
//        c = (char)x;
//        cipherText += c;
//        if(i >= cipherTextHex.length() - 2)break;
//        i += 2;
//    }

//    //
//    CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
//    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );
//    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( decryptedText ));
//    stfDecryptor.Put( reinterpret_cast<const unsigned char*>( cipherText.c_str() ), cipherText.size());

//    stfDecryptor.MessageEnd();

//    return decryptedText;
//}

//string AES::readCipher()
//{
//    ifstream in("/tmp/cipher.data");

//    string line;
//    string decryptedText;
//    while(getline(in, line))
//    {
//        if(line.length() > 1)
//        {
//            decryptedText += decrypt(line) + "\n";
//        }
//        line.clear();
//    }

//    cout<<"readCipher finish "<<endl;
//    in.close();

//    return decryptedText;
//}


