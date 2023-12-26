#include <cstdio>
#include <ctime>
#include <cstdlib>

#define  ULL unsigned long long // 64 bits

const int KeyBytes = 16; // 128 bits, 密钥长度
const int InputBytes = 1 << 24; // 2^24 bytes = 2^27 bits, 待加密的明文长度
const int SpnBytes = 8; // SPN的一次加密的分组大小

const int Nr = 8; // Nr轮
const ULL initVector = 0x3b9041e540cd23d0; // 初始向量

// 8位S盒, 置换大小: 0xFF
const ULL STable[256] = {
	248, 162, 88, 2, 38, 231, 108, 194, 4, 95, 85, 98, 13, 148,
	62, 201, 55, 15, 242, 28, 31, 69, 152, 135, 213, 0, 137, 178,
	37, 144, 39, 45, 216, 171, 149, 174, 238, 233, 239, 175, 121,
	77, 212, 63, 116, 8, 187, 215, 56, 151, 133, 50, 109, 35, 237,
	81, 94, 234, 79, 160, 74, 9, 164, 34, 190, 87, 26, 11, 211,
	70, 127, 49, 204, 255, 170, 72, 75, 224, 140, 43, 223, 147,
	141, 17, 202, 250, 113, 154, 210, 54, 245, 118, 82, 93, 236,
	155, 254, 92, 24, 179, 186, 29, 221, 232, 253, 73, 103, 57,
	41, 249, 25, 40, 91, 244, 18, 172, 59, 150, 198, 168, 247,
	101, 169, 1, 246, 192, 19, 159, 84, 14, 176, 208, 196, 251,
	71, 51, 89, 119, 126, 122, 65, 5, 188, 146, 114, 130, 99,
	106, 173, 228, 225, 229, 161, 100, 58, 220, 138, 86, 226,
	217, 157, 7, 52, 131, 222, 22, 129, 209, 96, 44, 64, 16, 120,
	136, 53, 139, 199, 90, 206, 185, 158, 181, 128, 61, 203, 125,
	252, 46, 21, 47, 230, 243, 165, 6, 193, 36, 153, 163, 30, 177,
	83, 182, 80, 23, 60, 156, 167, 97, 33, 20, 180, 3, 42, 145,
	111, 132, 115, 66, 142, 105, 197, 183, 27, 76, 107, 166, 241,
	207, 195, 205, 143, 189, 10, 218, 240, 200, 12, 112, 48, 235,
	117, 68, 123, 227, 124, 110, 102, 104, 67, 214, 184, 191, 78,
	134, 219, 32 
};

// 64的P盒
const int PTable[65] = {
	0,
	40, 38, 16, 50, 42, 23, 12, 6, 49, 51, 8, 5, 44, 7, 29, 63, 11,
	57, 13, 60, 4, 10, 64, 58, 59, 47, 24, 27, 17, 32, 34, 52, 46,
	21, 3, 53, 1, 30, 28, 54, 26, 18, 43, 36, 37, 14, 22, 25, 15,
	31, 45, 39, 19, 20, 41, 55, 33, 48, 2, 62, 56, 9, 61, 35
};

ULL k[Nr + 1]; // 轮密钥
ULL SPNPlus(ULL x, const ULL* k, const ULL* sBox);
void genKeys(const ULL *key, ULL* k);
ULL S(ULL u);
ULL P(ULL v);

int main() {
	ULL key[2]; // 两个64位的密钥=128位
	fread(key, KeyBytes, 1, stdin);

	genKeys(key, k);
	ULL x, y, IV = initVector; // 原文, 密文, 初始向量
	
	// CBC模式
	for (int i = 0; i < InputBytes / SpnBytes; ++i) {
		fread(&x, SpnBytes, 1, stdin);
		x ^= IV;
		y = SPNPlus(x, k, STable);
		fwrite(&y, SpnBytes, 1, stdout);
		IV = y;
	}
}

// 生成轮密钥
void genKeys(const ULL* key, ULL* k) {
	k[0] = key[0];
	k[1] = (key[0] << 8)  | (key[1] >> 56);
	k[2] = (key[0] << 16) | (key[1] >> 48);
	k[3] = (key[0] << 24) | (key[1] >> 40);
	k[4] = (key[0] << 32) | (key[1] >> 32);
	k[5] = (key[0] << 40) | (key[1] >> 24);
	k[6] = (key[0] << 48) | (key[1] >> 16);
	k[7] = (key[0] << 56) | (key[1] >> 8);
	k[8] = key[1];
}

// SPN加密主函数
ULL SPNPlus(ULL x, const ULL* k, const ULL* sBox) {
	ULL w = x, u, v;
	for (int i = 0; i < Nr - 1; i++) {
		u = k[i] ^ w;
		v = S(u);
		w = P(v);
	}
	u = k[Nr - 1] ^ w;
	v = S(u);
	return k[Nr] ^ v;
}

// S盒代换
inline ULL S(ULL u) {
	ULL res = 0;
	res |= STable[u & 0xff];
	u = u >> 8;
	res |= STable[u & 0xff] << 8;
	u = u >> 8;
	res |= STable[u & 0xff] << 16;
	u = u >> 8;
	res |= STable[u & 0xff] << 24;
	u = u >> 8;
	res |= STable[u & 0xff] << 32;
	u = u >> 8;
	res |= STable[u & 0xff] << 40;
	u = u >> 8;
	res |= STable[u & 0xff] << 48;
	u = u >> 8;
	res |= STable[u & 0xff] << 56;
	return res;
}

// P盒置换
inline ULL P(ULL v) {
	ULL res = 
		  (((v >> 24) & 1) << 63) | (((v >> 26) & 1) << 62) 
		| (((v >> 48) & 1) << 61) | (((v >> 14) & 1) << 60) 
		| (((v >> 22) & 1) << 59) | (((v >> 41) & 1) << 58) 
		| (((v >> 52) & 1) << 57) | (((v >> 58) & 1) << 56) 
		| (((v >> 15) & 1) << 55) | (((v >> 13) & 1) << 54)
		| (((v >> 56) & 1) << 53) | (((v >> 59) & 1) << 52) 
		| (((v >> 20) & 1) << 51) | (((v >> 57) & 1) << 50) 
		| (((v >> 35) & 1) << 49) | (((v >> 1)  & 1) << 48)
		| (((v >> 53) & 1) << 47) | (((v >> 7)  & 1) << 46) 
		| (((v >> 51) & 1) << 45) | (((v >> 4)  & 1) << 44) 
		| (((v >> 60) & 1) << 43) | (((v >> 54) & 1) << 42) 
		| (((v >> 0)  & 1) << 41) | (((v >> 6)  & 1) << 40)
		| (((v >> 5)  & 1) << 39) | (((v >> 17) & 1) << 38) 
		| (((v >> 40) & 1) << 37) | (((v >> 37) & 1) << 36) 
		| (((v >> 47) & 1) << 35) | (((v >> 32) & 1) << 34) 
		| (((v >> 30) & 1) << 33) | (((v >> 12) & 1) << 32) 
		| (((v >> 18) & 1) << 31) | (((v >> 43) & 1) << 30) 
		| (((v >> 61) & 1) << 29) | (((v >> 11) & 1) << 28) 
		| (((v >> 63) & 1) << 27) | (((v >> 34) & 1) << 26) 
		| (((v >> 36) & 1) << 25) | (((v >> 10) & 1) << 24) 
		| (((v >> 38) & 1) << 23) | (((v >> 46) & 1) << 22) 
		| (((v >> 21) & 1) << 21) | (((v >> 28) & 1) << 20) 
		| (((v >> 27) & 1) << 19) | (((v >> 50) & 1) << 18) 
		| (((v >> 42) & 1) << 17) | (((v >> 39) & 1) << 16) 
		| (((v >> 49) & 1) << 15) | (((v >> 33) & 1) << 14) 
		| (((v >> 19) & 1) << 13) | (((v >> 25) & 1) << 12) 
		| (((v >> 45) & 1) << 11) | (((v >> 44) & 1) << 10) 
		| (((v >> 23) & 1) << 9)  | (((v >> 9)  & 1) << 8) 
		| (((v >> 31) & 1) << 7)  | (((v >> 16) & 1) << 6)
		| (((v >> 62) & 1) << 5)  | (((v >> 2)  & 1) << 4) 
		| (((v >> 8)  & 1) << 3)  | (((v >> 55) & 1) << 2) 
		| (((v >> 3)  & 1) << 1)  | (((v >> 29) & 1) << 0); 
		return res;
}