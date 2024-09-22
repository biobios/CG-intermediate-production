#include "cglec.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define LEVEL_MAX 255

Image InitImage(int nx, int ny)
{
	Image img = { NULL, nx, ny };
	img.Data = (unsigned char*)malloc(sizeof(unsigned char) * nx * ny);
	if (img.Data == NULL)
		img.Nx = 0;
	return img;
}

ColorImage InitColorImage(int nx, int ny)
{
    ColorImage img = { NULL, NULL, NULL, nx, ny };
    img.DataR = (unsigned char*)malloc(sizeof(unsigned char) * nx * ny); //Rプレーン用メモリ
    img.DataG = (unsigned char*)malloc(sizeof(unsigned char) * nx * ny); //Gプレーン用メモリ
    img.DataB = (unsigned char*)malloc(sizeof(unsigned char) * nx * ny); //Bプレーン用メモリ
    if ((img.DataR == NULL) || (img.DataG == NULL) || (img.DataB == NULL))
        img.Nx = 0;     //メモリが一つでも確保できなかったらNxを0にして返す
    return img;
}

void ColorSetAll(ColorImage img, int R, int G, int B)
{
    Image img0 = { NULL, img.Nx, img.Ny };      //一時的なグレイスケール画像
    img0.Data = img.DataR;  CglSetAll(img0, R); //Rプレーンのデータに明度Rを設定
    img0.Data = img.DataG;  CglSetAll(img0, G); //Gプレーンのデータに明度Gを設定
    img0.Data = img.DataB;  CglSetAll(img0, B); //Bプレーンのデータに明度Bを設定
}

void SaveColorImage(ColorImage img, const char* fname)
{
    Image imgR = { img.DataR, img.Nx, img.Ny }; //グレイスケールのRプレーン
    Image imgG = { img.DataG, img.Nx, img.Ny }; //グレイスケールのGプレーン
    Image imgB = { img.DataB, img.Nx, img.Ny }; //グレイスケールのBプレーン
    CglSaveColorBMP(imgR, imgG, imgB, fname);
}

void FreeColorImage(ColorImage img)
{
    free(img.DataR);
    free(img.DataG);
    free(img.DataB);
}

struct Complex
{
	double re;
	double im;
};

Complex ComplexAdd(const Complex a, const Complex b)
{
	Complex c = { a.re + b.re, a.im + b.im };
	return c;
}

Complex ComplexMul(const Complex a, const Complex b)
{
	Complex c = { a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re };
	return c;
}

void MaskToColorImage(const Image mask, ColorImage img)
{

    int y_max = mask.Ny;
    int x_max = mask.Nx;

    if(img.Nx < x_max) x_max = img.Nx;
    if(img.Ny < y_max) y_max = img.Ny;

    for (int y = 0; y < y_max; y++) {
        for (int x = 0; x < x_max; x++) {
			int n = mask.Data[y * mask.Nx + x];
            double rate = (double)n / LEVEL_MAX;
            *(img.DataR + y * img.Nx + x) *= rate;
            *(img.DataG + y * img.Nx + x) *= rate;
            *(img.DataB + y * img.Nx + x) *= rate;
		}
	}
}


#define D 1000
#define N 1000
#define ZOOM 2.0

int main()
{
    ColorImage img = InitColorImage(D, D);
    Image mask = InitImage(D, D);

    if (img.Nx == 0 || mask.Nx == 0) {
        printf("メモリ確保エラー\n");
        return 1;
    }

    // ジュリア集合の描画
    Complex c = { -0.8, 0.156 };

    int center_x = D / 2;
    int center_y = D / 2;

    for (int y = 0; y < D; y++) {
        for (int x = 0; x < D; x++) {
			Complex z = { (double)(x - center_x) / (D * ZOOM), (double)(y - center_y) / (D * ZOOM)};
			int n;
            for (n = 0; n < N; n++) {
                z = ComplexAdd(ComplexMul(z, z), c);

                if (z.re * z.re + z.im * z.im > 4.0) break;
			}
            double t = (double)n / N;

            // 0.6乗することで、明るい部分を強調
            int R = LEVEL_MAX * pow(t, 0.6);

            mask.Data[y * D + x] = R;

		}
	}


    // 単一色画像の作成
    ColorSetAll(img, 90, 181, 178);
    // マスクをカラー画像に適用
    MaskToColorImage(mask, img);
    // 画像の保存
    SaveColorImage(img, "julia1.bmp");

    // グラデーションの作成
    ColorSetAll(img, 0, 0, 0);
    for (int y = 0; y < D; y++) {
        for (int x = 0; x < D; x++) {
			img.DataR[y * D + x] = 255 * (2 * D - y - x) / (2 * D);
            img.DataB[y * D + x] = 255 * (y + x) / (2 * D);
		}
	}

    // マスクをカラー画像に適用
    MaskToColorImage(mask, img);

    // 画像の保存
    SaveColorImage(img, "julia2.bmp");

    // メモリの解放
	FreeColorImage(img);
	free(mask.Data);
	return 0;
}