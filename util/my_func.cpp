#include "my_func.h"
vector<string> my_func::explode(const string &delimiter, const string &str){
    int len = str.length();
    int dlen = delimiter.length();
    vector<string> arr;
    if (0 == dlen)
        return arr;

    int i = 0, k = 0;
    
    for (i = 0; i < len; ++i){
        if (str.substr(i, dlen) != delimiter)
            continue;
        arr.push_back(str.substr(k, i - k));
        i += dlen;
        k = i;
    }
    arr.push_back(str.substr(k, i - k));
    return arr;
}

int my_func::get_day_cnt(int st){
    time_t now;
    time(&now);
    //新鲜购的逻辑时间起点为10点,减去10小时刚为自然时间的零点
    return my_func::diff_day(st - 10 * 3600, now - 10 * 3600);
}

int my_func::get_ts_before_n_day(int day){
    time_t now;
    time(&now);
    return now - (day * 86400);
}

/*
 *获得当天结束的时间戳
 * */
time_t my_func::day_end_ts(){
    time_t now;
    time(&now);
    tm *tm_p = localtime(&now);
    tm tm_t;
    char str[80];
    const char* format1 = "%Y-%m-%d 23:59:59";
    const char* format2 = "%Y-%m-%d %H:%M:%S";
    strftime(str, 80, format1, tm_p);
    if(strptime(str, format2, &tm_t) == NULL)
        return 0;
    return mktime(&tm_t);
}
/*
 *判断间隔几个自然日
 * */
int my_func::diff_day(time_t ts_start, time_t ts_end) {
    tm *tm_start, *tm_end;
    struct tm tm_s, tm_e;
    tm_start = localtime(&ts_start);
    char str[80];
    const char* format1 = "%Y-%m-%d 00:00:00";
    const char* format2 = "%Y-%m-%d %H:%M:%S";

    strftime(str, 80, format1, tm_start);
    if(strptime(str, format2, &tm_s) == NULL)
        return 0;
    ts_start = mktime(&tm_s);

    tm_end = localtime(&ts_end);
    strftime(str, 80, format1, tm_end);
    if(strptime(str, format2, &tm_e) == NULL)
        return 0;
    ts_end = mktime(&tm_e);

    int res = (ts_end-ts_start)/86400;
    return res;
}


template <class T>
vector<pair<int, int> > my_func::locate_number(vector<T> v, 
        unsigned int n){
    vector<pair<int, int> > v_rate;
    int start = 0, end = -1;
    for(size_t i = 0; i < v.size(); i++){
        start = end + 1; 
        end += (int)(v[i] * n);
        v_rate.push_back(make_pair(start, end));
    }
    return v_rate;
}

vector<pair<int, int> > my_func::locate_number(vector<string> v, 
        unsigned int n){
    vector<float> v_float;
    for(size_t i = 0; i < v.size(); i++){
        float rate = atof(v[i].c_str());
        v_float.push_back(rate);
    }
    return my_func::locate_number(v_float, n);
}

bool my_func::between_pair(pair<int, int> p, int n){
    return n >= p.first && n <= p.second;
}

/*
int my_func::binary_search(int *arr, int size, int key){
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
}
*/
