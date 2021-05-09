#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "matrix.h"

// ----------------------------------------------------------------------------
// 行列演算用関数群
// ----------------------------------------------------------------------------

// mat_alloc:行列要素用のメモリを確保する
bool mat_alloc(matrix *mat, int rows, int cols)
{
  if(rows > 0 && cols > 0){
    mat->rows = rows;
    mat->cols = cols;
    mat->elems = (double*)calloc((rows+1) * (cols+1),sizeof(double));
    if(mat->elems == NULL){
      printf("[ ERROR ] メモリを確保できませんでした\n");
    }
  }else{
    return false;
  }
  return true;
}

// mat_free:使い終わった行列のメモリを解放する
void mat_free(matrix *mat)
{
  free(mat->elems);
  mat->rows = 0;
  mat->cols = 0;
  mat->elems = NULL;
}

// mat_print:行列の中身を表示する
void mat_print(matrix mat)
{
    if (mat.rows == 0 || mat.cols == 0 || mat.elems == NULL) {
        fprintf(stderr, "Matrix is NULL or zero size!\n");
        return;
    }

    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            printf("%6.4f%s", mat_elem(mat, i, j), (j == mat.cols - 1) ? "\n" : "  ");
        }
    }
}

// mat_copy:srcの中身を*dstにコピーする
bool mat_copy(matrix *dst, matrix src)
{
  if(dst->cols != src.cols){
    return false;
  }
  if(dst->rows != src.rows){
    return false;
  }
  for(int j = 0; j < src.cols; j++){
    for(int i = 0; i < src.rows; i++){
      mat_elem(*dst, i, j) = mat_elem(src, i, j) ;
    }
  }
  return true;
}

// mat_add:mat2+mat3を*mat1に代入する
bool mat_add(matrix *res, matrix mat1, matrix mat2)
{
  for(int j = 0; j < mat1.cols; j++){
    for(int i = 0; i < mat1.rows; i++){
      mat_elem(*res, i, j) = mat_elem(mat1, i, j) + mat_elem(mat2, i, j) ;
    }
  }
  return true;
}

// mat_sub:mat2-mat3を*mat1に代入する
bool mat_sub(matrix *res, matrix mat1, matrix mat2)
{
  for(int i = 0; i < mat1.rows; i++){
    for(int j = 0; j < mat1.cols; j++){
      mat_elem(*res, i, j) = mat_elem(mat1, i, j) - mat_elem(mat2, i, j) ;
    }
  }
  return true;
}

// mat_mul:mat2とmat3の行列積を*mat1に代入する
bool mat_mul(matrix *res, matrix mat1, matrix mat2)
{
  if(mat1.cols != mat2.rows){
    return false;
  }
  if(mat1.rows != res->rows || mat2.cols != res->cols){
    return false;
  }
  matrix mat3;
  mat_alloc(&mat3,res->rows,res->cols);

  for(int i = 0;i < mat1.rows; i++){
    for(int k = 0; k < mat2.cols; k++){
      for(int j = 0; j < mat2.rows; j++){
        mat_elem(mat3, i, k) += mat_elem(mat1, i, j) * mat_elem(mat2, j, k) ;
      }
    }
  }
  mat_copy(res,mat3);
  return true;
}


// mat_muls:mat2をc倍（スカラー倍）した結果を*mat1に代入する
bool mat_muls(matrix *res, matrix mat, double c)
{
  for(int i = 0; i < mat.rows; i++){
    for(int j = 0; j < mat.cols; j++){
      mat_elem(*res, i, j) = mat_elem(mat, i, j) * c ;
    }
  }
  return true;
}

// mat_trans:mat2の転置行列を*mat1に代入する
bool mat_trans(matrix *res, matrix mat)
{
  if(res->cols != mat.rows){
    return false;
  }
  if(mat.cols != res->rows){
    return false;
  }
  matrix mat3;
  mat_alloc(&mat3,res->rows,res->cols);
  for(int i = 0; i < mat.cols; i++){
    for(int j = 0; j < mat.rows; j++){
      mat_elem(mat3, i, j) = mat_elem(mat, j, i) ;
    }
  }
  mat_copy(res,mat3);
  return true;
}

// mat_unit:単位行列を与える
bool mat_ident(matrix *mat)
{
  if(mat->cols != mat->rows){
    return false;
  }
  for(int i = 0; i < mat->rows; i++){
    mat_elem(*mat, i, i) = 1 ;
  }
  return true;
}

// mat_equal:mat1とmat2が等しければtrueを返す
bool mat_equal(matrix mat1, matrix mat2)
{
  if(mat1.cols != mat2.cols){
    return false;
  }
  if(mat1.rows != mat2.rows){
    return false;
  }
  for(int i = 0; i < mat1.rows; i++){
    for(int j = 0; j < mat1.cols; j++){
      const double a = mat_elem(mat1, i, j);
      const double b = mat_elem(mat2, i, j);
      if(fabs(a - b) >1.0e-12){
        return false;
      }
    }
  }
  return true;
}

// mat_solve:連立一次方程式 ax=b を解く．ピボット選択付き
bool mat_solve(matrix *x, matrix A, matrix b)
{
    int i, j, k;
    matrix A2, b2;

    // 行列のサイズチェック。この例では、bとxを列ベクトルに限定しているが、
    // この部分は後ほど改良する
    if (A.rows != A.cols) return false;
    if (A.cols != b.rows) return false;
    if ((x->rows != b.rows) || (x->cols != b.cols)) return false;

    // 行列aと行列bの値を書き換えないよう、別の行列を用意する
    if (!mat_alloc(&A2, A.rows, A.cols)) return false;
    if (!mat_alloc(&b2, b.rows, b.cols)) return false;

    // 用意した行列にAとbの値をコピーする。以下、これらの行列を用いて計算する
    mat_copy(&A2, A);
    mat_copy(&b2, b);

    /*
     * ガウスの消去法：
     * 普通に作れば10行程度。 forループを3つ使う？
     * 行列式がゼロかどうかの判定も忘れないこと
     */
     for(int i=0; i<A2.rows-1; i++){
       for(int j=1; j<A2.rows-i; j++){
         if(fabs(mat_elem(A2,i,i))<fabs(mat_elem(A2,i+j,i))){
           for(int k=i; k<A2.cols; k++){
             double q = mat_elem(A2,i,k);
             mat_elem(A2,i,k) = mat_elem(A2,i+j,k);
             mat_elem(A2,i+j,k) = q;
           }
           for(int k=0; k<b2.cols; k++){
             double q = mat_elem(b2,i,k);
             mat_elem(b2,i,k) = mat_elem(b2,i+j,k);
             mat_elem(b2,i+j,k) = q;
           }
         }
       }
       for(int j=i+1; j<A2.rows; j++){
         double p = mat_elem(A2,j,i)/mat_elem(A2,i,i);
         for(int h=0; h<b2.cols; h++){
           mat_elem(b2,j,h) = mat_elem(b2,j,h)-mat_elem(b2,i,h)*p;
         }
         for(int k=i; k<A2.cols; k++){
           mat_elem(A2,j,k) = mat_elem(A2,j,k)-mat_elem(A2,i,k)*p;
         }
       }
     }
     double s = 1;
     for(int i=0; i<A2.cols; i++){
       s *= mat_elem(A2,i,i);
     }
     if(s != s) return false;
    /*
     *  後退代入：
     *  普通に作れば5-7行程度。 forループを2つ使う？
     */
     for(int h=0; h<b2.cols; h++){
       for(int i=0; i<A2.cols; i++){
         mat_elem(b2,A2.cols-i-1,h) = mat_elem(b2,A2.cols-i-1,h)/mat_elem(A2,A2.cols-i-1,A2.cols-i-1);
        for(int j=A2.cols-i; j<A2.cols; j++){
          mat_elem(b2,A2.cols-i-1,h) -= mat_elem(A2,A2.cols-i-1,j)*mat_elem(b2,j,h)/mat_elem(A2,A2.cols-i-1,A2.cols-i-1);
        }
       }
     }
    // 結果を x にコピー
    mat_copy(x, b2);

    // 終わったらメモリを解放
    mat_free(&A2);
    mat_free(&b2);

    return true;
}

// mat_inverse:行列Aの逆行列を*invAに与える
bool mat_inverse(matrix *invA, matrix A)
{
  matrix I;
  mat_alloc(&I, A.rows, A.cols);
  mat_ident(&I);
  matrix A2;
  mat_alloc(&A2, A.rows, A.cols);
  mat_copy(&A2, A);
  mat_solve(invA, A2, I);
  if(mat_solve(invA, A2, I) == false) return false;

  return true;
}
