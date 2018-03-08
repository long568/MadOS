// const MadInt Int2Str_TAB[] = {
//     1000000000,
//     100000000,
//     10000000,
//     1000000,
//     100000,
//     10000,
//     1000,
//     100,
//     10,
//     1
// };

// static MadU8 Int2Str(MadU8 * dst, MadInt val)
// {
//     MadU8 i, j;
//     MadU8 f, t;

//     if(val == 0) {
//     		*dst = '0';
//     		return 1;
//     }

//     j = 0;
//     f = MFALSE;
//     for(i=0; i<10; i++) {
//         t = val / Int2Str_TAB[i];
//         if(t != 0) f = MTRUE;
//         if(f == MTRUE) {
//             dst[j++] = t + '0';
//             val = val % Int2Str_TAB[i];
//         }
//     }
//     return j;
// }

//             int i;
//             MadU32 len;
//             const MadU8 d_head[] = "uIP -> Acked["; // 13
//             const MadU8 d_midd[] = "], Rexmit[";    // 10
//             const MadU8 d_tail[] = "]";             // 1
            
//             len = 24;
//             for(i=0; i<13; i++) {
//                 *ack_str++ = d_head[i];
//             }
//             i = Int2Str(ack_str, cnt_acked);
//             len += i;
//             ack_str += i;
//             for(i=0; i<10; i++) {
//             		*ack_str++ = d_midd[i];
//             }
//             i = Int2Str(ack_str, cnt_rexmit);
//             len += i;
//             ack_str += i;
//             for(i=0; i<1; i++) {
//             		*ack_str++ = d_tail[i];
//             }