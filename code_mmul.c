// matrix mul a[m][k] * b[k][n]
template <unsigned M, unsigned K, unsigned N>
void mmul_blocked(unsigned rowA, unsigned colA, unsigned colB,
                  const int16 * __restrict pA, const int16 * __restrict pB, int16 * __restrict pC) // pA,pb,pc: con tro tro den cac matran 
{
   using MMUL = aie::mmul<M, K, N, int16, int16>; // dinh nghia kieu MMUL voi kich thuoc M, K, N
 
   for (unsigned z = 0; z < rowA; z += 2) chess_loop_range(2,) // 2 dong 1 lan 
       // tim den dia chi 0 (dau tien) cua 2 dong dang tinh trong C 
       int16 * __restrict pC1 = pC + (      z * colB +       0) * MMUL::size_C; 
       int16 * __restrict pC2 = pC + ((z + 1) * colB +       0) * MMUL::size_C;

       for (unsigned j = 0; j < colB; j += 2) chess_loop_range(2,) { // 2 cot 1 lan 
           // MMUL::size_A: so luong phan tu trong 1 block ma phep nhan ma tran co the xu ly cung luc (< so voi kich thuoc cua A)
           const int16 * __restrict pA1 = pA + (      z * colA +       0) * MMUL::size_A; // Tinh con tro den vi tri bat dau tinh cua 1 block (1 chieu) can xu ly cua A 
           const int16 * __restrict pA2 = pA + ((z + 1) * colA +       0) * MMUL::size_A; // same nhung la hang +1 
           const int16 * __restrict pB1 = pB + (      0 * colB +       j) * MMUL::size_B; // same as A but cot 
           const int16 * __restrict pB2 = pB + (      0 * colB + (j + 1)) * MMUL::size_B; // same as A but cot 

           // load du lieu tu pa12, pb12 o tren, cung voi cap nhat vi tri ngay sau do 
           aie::vector<int16, MMUL::size_A> A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A;  // load ca block voi do rong size_A vao A0, sau do dich pA1 bang 1 block 
           aie::vector<int16, MMUL::size_A> A1 = aie::load_v<MMUL::size_A>(pA2); pA2 += MMUL::size_A;  // same but hang duoi 
           aie::vector<int16, MMUL::size_B> B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * colB; // same but cot 
           aie::vector<int16, MMUL::size_B> B1 = aie::load_v<MMUL::size_B>(pB2); pB2 += MMUL::size_B * colB; // same but cot +1 

           // nhan 2 vector 1 chieu => tao ra 1 vector ket qua bang 1 block 
           MMUL C00; C00.mul(A0, B0);  
           MMUL C01; C01.mul(A0, B1);
           MMUL C10; C10.mul(A1, B0);
           MMUL C11; C11.mul(A1, B1);

           for (unsigned i = 1; i < colA; ++i) chess_prepare_for_pipelining chess_loop_range(3,) {
               // tiep tuc load 
               A0 = aie::load_v<MMUL::size_A>(pA1); pA1 += MMUL::size_A; 
               A1 = aie::load_v<MMUL::size_A>(pA2); pA2 += MMUL::size_A;
               B0 = aie::load_v<MMUL::size_B>(pB1); pB1 += MMUL::size_B * colB;
               B1 = aie::load_v<MMUL::size_B>(pB2); pB2 += MMUL::size_B * colB;

               // nhan a0, b0 roi cong don vao c00 
               C00.mac(A0, B0);
               C01.mac(A0, B1);
               C10.mac(A1, B0);
               C11.mac(A1, B1);
           }
           // Luu ket qua 4 khoi C00, C01, C10, C11 tu accumulator -> vector -> bo nho.
           aie::store_v(pC1, C00.template to_vector<int16>()); pC1 += MMUL::size_C;
           aie::store_v(pC1, C01.template to_vector<int16>()); pC1 += MMUL::size_C;
           aie::store_v(pC2, C10.template to_vector<int16>()); pC2 += MMUL::size_C;
           aie::store_v(pC2, C11.template to_vector<int16>()); pC2 += MMUL::size_C;
       }
   }
}
