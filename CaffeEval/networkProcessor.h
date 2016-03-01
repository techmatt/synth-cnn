
class NetworkProcessor
{
public:
    void init();
    void evaluateAllUsers(const NetflixDatabase &database);
    void evaluateAllUsers(const NetflixDatabase &database, const vector<Rating> &ratings, const string &filename);
    void outputUsers(const string &filename) const;

    double evaluateRating(const NetflixDatabase &database, const Rating &rating);

private:
    
    Netf net;
};
