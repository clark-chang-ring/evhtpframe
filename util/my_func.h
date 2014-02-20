#ifndef MY_FUNC_H_
#define MY_FUNC_H_
#include<vector>
#include<string>
#include<cmath>
using namespace std;
class my_func{
    public:
        static vector<string> explode(const string &delimiter, 
                const string &str);
        /*
         *输入概率，得到其在分布区间n 对应的区间
         *例 输入{"0.2","0.7","0.1"}, 100 输出{{0,19},{20,89},{90,99}}
         * */
        template<class T>
        static vector<pair<int, int> > locate_number(vector<T>, 
                unsigned int n);
        /*特例化string*/
        static vector<pair<int, int> > locate_number(vector<string>, 
                unsigned int n);
        static int get_ts_before_n_day(int day);
        static int get_day_cnt(int st);
        static bool between_pair(pair<int, int>, int n);
        static int diff_day(time_t datetime_now, time_t datetime_before);
        static time_t day_end_ts();
        template<class T1>
        static int binary_search(T1 *arr, int size, T1 key){
            int low = 0, high = size - 1, mid;
            while(low <= high){
                mid = (low + high) / 2;
                if(key == arr[mid])
                    return mid;
                if(key < arr[mid])
                    high = mid - 1;
                if(key > arr[mid])
                    low = mid + 1;
            }
            return -1;
        };
//        int write_log() 
        /*
        static int binary_search(int *arr, int size, int key);
        */
};
#endif
