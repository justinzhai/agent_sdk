#include "curl.h"
#include "string"

using namespace std;
CURLcode curl_post_req(const string &url, const string &postParams, string &response);