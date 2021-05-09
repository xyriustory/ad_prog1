#ifndef _MATRIX_H
#define _MATRIX_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define FROM0
// 要素を交換するマクロ
#define swap(a, b) do { \
    double t = (a);     \
    a = b;              \
    b = t;              \
} while( 0 );

/*
 * 行列用構造体
 * rows: 行数
 * cols: 列数
 * elems: 行列要素を入れた一次元配列
 */
typedef struct {
	int rows;
	int cols;
	double *elems;
} matrix;

// 行列要素を取得するマクロ
#define mat_elem(m, i, j) (m).elems[(i) * (m).cols + (j)]

// ----------------------------------------------------------------------------
// プロトタイプ宣言
// ----------------------------------------------------------------------------

bool mat_alloc(matrix *mat, int rows, int cols);
void mat_free(matrix *mat);
void mat_print(matrix mat);
bool mat_copy(matrix *dst, matrix src);
bool mat_add(matrix *res, matrix mat1, matrix mat2);
bool mat_sub(matrix *res, matrix mat1, matrix mat2);
bool mat_mul(matrix *res, matrix mat1, matrix mat2);
bool mat_muls(matrix *res, matrix mat, double c);
bool mat_trans(matrix *res, matrix mat);
bool mat_ident(matrix *mat);
bool mat_equal(matrix mat1, matrix mat2);
bool mat_solve(matrix *x, matrix A, matrix b);
bool mat_inverse(matrix *invA, matrix A);

#endif
