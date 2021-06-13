#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

using std::bind;
using std::enable_shared_from_this;
using std::lock_guard;
using std::make_shared;
using std::mutex;
using std::placeholders::_1;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::weak_ptr;

struct Stock {
	string key;
	Stock(const string& k) : key(k) {}
};

class StockFactory : public enable_shared_from_this<StockFactory> {
 public:
 	shared_ptr<Stock> GetStock(const string& key) {
 		lock_guard<mutex> lck(mtx_);
 		weak_ptr<Stock>& wkStock = stocks_[key];
 		shared_ptr<Stock> pStock = wkStock.lock();
 		if (pStock == nullptr) {
 			pStock.reset(new Stock(key),
 									 bind(&StockFactory::WeakDeleteCallback,
 									 			weak_ptr<StockFactory>(shared_from_this()),
 									 			_1));
 			wkStock = pStock;
 		}
 		return pStock;
 	}

 private:
 	static void WeakDeleteCallback(const weak_ptr<StockFactory>& wkFactory,
 																	Stock *stock) {
 		// Consider the situation where StockFactory has been deleted, while stocks not.
 		shared_ptr<StockFactory> factory(wkFactory);
 		if (factory != nullptr) {
 			factory->RemoveStock(stock);
 		}
 		delete stock;
 	}

 	void RemoveStock(Stock *stock) {
 		assert(stock != nullptr);
 		lock_guard<mutex> lck(mtx_);
 		stocks_.erase(stock->key);
 	}

 private:
 	mutable mutex mtx_;
 	unordered_map<string, weak_ptr<Stock>> stocks_;
};

int main(int argc, char **argv) {
	shared_ptr<StockFactory> factory = make_shared<StockFactory>();
	shared_ptr<Stock> stock1 = factory->GetStock("HJ");
	shared_ptr<Stock> stock2 = factory->GetStock("HJ");
	assert(stock1 == stock2);
}