#include"definitions.h"

stock* initialize_stock_values(string directory, int& n_stock, int& error_code) {
    ifstream data(directory);

    if (!data.is_open()) {
        cout << "Failed to read file" << endl;
        error_code = -1;
        return nullptr;
    }

    // Skip the header
    string header;
    getline(data, header);

    // Count the number of lines
    n_stock = 0;
    string line;
    while (getline(data, line)) {
        n_stock++;
    }

    data.clear();  // Clear the EOF flag
    data.seekg(0); // Go back to the beginning of the file
    getline(data, header); // Skip the header again

    stock* vettore = new stock[n_stock];

    if (vettore == nullptr) {
        error_code = -2;
        return nullptr;
    }

    int index = 0;
    while (getline(data, line) && index < n_stock) {
        stringstream ss(line);
        string temp;
        
        getline(ss, vettore[index].date, ',');
        getline(ss, temp, ',');
        vettore[index].open = stod(temp);
        getline(ss, temp, ',');
        vettore[index].high = stod(temp);
        getline(ss, temp, ',');
        vettore[index].low = stod(temp);
        getline(ss, temp, ',');
        vettore[index].close = stod(temp);
        getline(ss, temp, ',');
        vettore[index].volume = stod(temp);
        getline(ss, temp, ',');
        vettore[index].dividends = stod(temp);
        getline(ss, temp, ',');
        vettore[index].stock_splits = stod(temp);

        index++;
    }

    data.close();
    error_code = 0;
    return vettore;
}

void print_data (stock* stocks, int n_stock , int& error_code){
    if (error_code == 0) {
        // Process the stock data
        for (int i = 0; i < n_stock; i++) {
            cout << "Date: " << stocks[i].date << " Open: " << stocks[i].open << " High: " << stocks[i].high
                 << " Low: " << stocks[i].low << " Close: " << stocks[i].close << " Volume: " << stocks[i].volume
                 << " Dividends: " << stocks[i].dividends << " Stock Splits: " << stocks[i].stock_splits << endl;
        }
    } else {
        cout << "Error code: " << error_code << endl;
    }
}

weighted_avg* initialize_weighted_avgs(stock* data, int n_dati, double beta) {
    weighted_avg* weighted_avgs = new weighted_avg[n_dati];
    double sum_squared_deviations = 0.0;

    weighted_avgs[0].value = data[0].open;  // Initial value should be the first data point
    weighted_avgs[0].sigma = 0;  // Initial sigma is 0
    
    for (int i = 1; i < n_dati; ++i) {
        weighted_avgs[i].value = beta * weighted_avgs[i - 1].value + (1 - beta) * data[i].open;
        double deviation = data[i].open - weighted_avgs[i].value;
        sum_squared_deviations += deviation * deviation;
        weighted_avgs[i].sigma = sqrt(sum_squared_deviations / (i+1)); 
    }
    return weighted_avgs;
}

void print_weighted_avgs (weighted_avg* weighted_avgs, int n_dati) {
    for (int i = 0; i < n_dati; ++i) {
        printf("Weighted Average %d: Value = %f, Sigma = %f\n", i, weighted_avgs[i].value, weighted_avgs[i].sigma);
    }
}

void save_data_to_file(stock* data, weighted_avg* weighted_avgs , portfolio asset, int n_dati, string filename) {
    ofstream file(filename);

    if (!file.is_open()) {
        cout << "Failed to open file for writing" << endl;
        return;
    }

    file << "Index,Stock Value,Weighted Average,Sigma,Portfolio Value\n";
    for (int i = 0; i < n_dati; ++i) {
        file << i << ',' << data[i].open << ',' << weighted_avgs[i].value << "," << weighted_avgs[i].sigma <<"," << asset.value[i]<< '\n';
    }

    file.close();
}

void simulate_portfolio(portfolio& asset, stock* stocks, weighted_avg* weighted_avgs, float s_index, float percent, int n_stock) {
    double invested = 0;
    double not_invested = asset.value[0];  // Initially, all the value is not invested
    asset.n_stocks[0]= 0;
    for (int i = 0; i < n_stock; i++) {
        // Check for buy signal
        if (weighted_avgs[i].value - s_index * weighted_avgs[i].sigma > stocks[i].open) {
            double amount_to_invest = percent * not_invested;
            double number_of_shares = amount_to_invest / stocks[i].open;

            if (not_invested >= amount_to_invest) {
                // Update portfolio for buy action
                asset.n_stocks[i] = number_of_shares;
                invested += amount_to_invest;
                not_invested -= amount_to_invest;
            } else {asset.n_stocks[i]= 0;}
        }
        // Check for sell signal
        else if (weighted_avgs[i].value + s_index * weighted_avgs[i].sigma < stocks[i].open) {
            double amount_to_divest = percent * invested;
            double number_of_shares = amount_to_divest / stocks[i].open;

            if (asset.n_stocks[i] >= number_of_shares) {
                // Update portfolio for sell action
                asset.n_stocks[i] = - number_of_shares;
                invested -= amount_to_divest;
                not_invested += amount_to_divest;
            } else {asset.n_stocks[i]= 0;}
        }

        // Calculate the total value of the portfolio for this period
        double total_shares = 0;
        for (int j = 0; j <= i; j++) {
            total_shares += asset.n_stocks[j] ;
        }
        asset.value[i] = not_invested + total_shares*stocks[i].open;
    }
}
