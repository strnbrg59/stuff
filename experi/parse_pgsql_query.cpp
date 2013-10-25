#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <iostream>

using namespace std;

string g_query = "SELECT CLASS_ID AS CID, SUBCLASS_ID AS SCID, 300 AS FX_GR, TARGET as FX_T, TIMESTAMP, SUM(DATA_0)::bigint AS DATA_0, SUM(DATA_1)::bigint AS DATA_1, SUM(DATA_2)::bigint AS DATA_2, SUM(DATA_3 )::bigint AS DATA_3 FROM stats_300 WHERE CLASS_ID = 1 AND SUBCLASS_ID = 0 AND TIMESTAMP > 1367596800 AND TIMESTAMP <= 1367600000  GROUP BY CID, SCID, FX_GR, FX_T, TIMESTAMP;";

struct TimestampRange
{
    TimestampRange(uint start, uint end) : start_(start), end_(end) {}
    uint start_;
    uint end_;
};

ostream& operator<<(ostream& out, TimestampRange const& tr)
{
    out << "(" << tr.start_ << ", " << tr.end_ << ")";
    return out;
}

TimestampRange parse_timestamp_range(string query)
{
    typedef boost::tokenizer< boost::char_separator<char> > TokenizerT;
    TokenizerT nizer(query, boost::char_separator<char>(" "));
    bool found_start=false, found_end=false;
    TokenizerT::const_iterator iter = nizer.begin();
    TimestampRange result(0,0);
    char* endptr = NULL;
    while ((!found_end) && (iter != nizer.end())) {
        if (*iter == "TIMESTAMP") {
            ++iter;
            assert(iter != nizer.end());
            assert((*iter == ">") || (*iter == "<="));

            ++iter;
            assert(iter != nizer.end());
            uint val = strtoul(iter->c_str(), &endptr, 10);
            cerr << "strtoul(" << iter->c_str() << ")\n";
            assert(!*endptr);
            if (!found_start) {
                result.start_ = val;
                found_start = true;
            } else {
                result.end_ = val;
                found_end = true;
            }
        }
        
        ++iter;
    }

    return result;
}

int main()
{
    TimestampRange tr = parse_timestamp_range(g_query);
    cout << tr << '\n';
}
