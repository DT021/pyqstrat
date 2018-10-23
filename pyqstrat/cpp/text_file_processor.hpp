#ifndef __text_file_processor_hpp
#define __text_file_processor_hpp

#include <string>
#include <stdexcept>
#include <regex>
#include <cmath>
#include <istream>
#include <fstream>
#include <boost/iostreams/filtering_stream.hpp>

#include "pq_types.hpp"



struct PriceQtyMissingDataHandler :  public MissingDataHandler {
    //PriceQtyMissingDataHandler() {}
    void call(std::shared_ptr<Record> record) override;
};

class PrintBadLineHandler : public BadLineHandler {
public:
    PrintBadLineHandler(bool raise = false);
    std::shared_ptr<Record> call(int line_number, const std::string& line, const std::exception& ex) override;
private:
    bool _raise;
};

class RegExLineFilter : public LineFilter {
public:
    RegExLineFilter(const std::string& pattern);
    bool call(const std::string& line) override;
private:
    std::regex _pattern;
};

class SubStringLineFilter : public LineFilter {
public:
    SubStringLineFilter(const std::vector<std::string>& patterns);
    bool call(const std::string& line) override;
private:
    std::vector<std::string> _patterns;
};

class IsFieldInList : public CheckFields {
public:
    IsFieldInList(int flag_idx, const std::vector<std::string>& flag_values);
    bool call(const std::vector<std::string>& fields) override;
private:
    int _flag_idx;
    std::vector<std::string> _flag_values;
};

class StreamHolder {
public:
    inline StreamHolder(std::shared_ptr<boost::iostreams::filtering_streambuf<boost::iostreams::input>> buf, std::shared_ptr<std::istream> file, std::shared_ptr<std::istream> istream) :
    _buf(buf),
    _file(file),
    _istream(istream) {}
    bool operator()(std::string& line) {
        std::istream& istr = std::getline(*_istream, line);
        if (istr) return true;
        return false;
    }
    virtual ~StreamHolder();
private:
    std::shared_ptr<boost::iostreams::filtering_streambuf<boost::iostreams::input>> _buf;
    std::shared_ptr<std::istream> _file;
    std::shared_ptr<std::istream> _istream;
};

struct TextFileDecompressor : public Function<std::shared_ptr<StreamHolder>(const std::string&, const std::string&)> {
    std::shared_ptr<StreamHolder> call(const std::string& filename, const std::string& compression) override;
};

class TextFileProcessor : public FileProcessor {
public:
    TextFileProcessor(
                      RecordGenerator* record_generator,
                      LineFilter* line_filter,
                      RecordParser* record_parser,
                      BadLineHandler* bad_line_handler,
                      RecordFilter* record_filter,
                      MissingDataHandler* missing_data_handler,
                      QuoteAggregator* quote_aggregator,
                      TradeAggregator* trade_aggregator,
                      OpenInterestAggregator* open_interest_aggregator,
                      OtherAggregator* other_aggregator,
                      int skip_rows);

    int call(const std::string& input_filename, const std::string& compression);
private:
    Function<std::shared_ptr<StreamHolder>(const std::string&, const std::string&)>* _record_generator;
    Function<bool (const std::string&)>* _line_filter;
    Function<std::shared_ptr<Record> (const std::string&)>* _record_parser;
    Function<std::shared_ptr<Record> (int, const std::string&, const std::exception&)>* _bad_line_handler;
    Function<bool (const Record&)>* _record_filter;
    Function<void (std::shared_ptr<Record>)>* _missing_data_handler;
    Function<void (const QuoteRecord&, int)>* _quote_aggregator;
    Function<void (const TradeRecord&, int)>* _trade_aggregator;
    Function<void (const OpenInterestRecord&, int)>* _open_interest_aggregator;
    Function<void (const OtherRecord&, int)>* _other_aggregator;
    int _skip_rows;
};

#endif
