static int const maxlinelen = 80;

struct Printable
{
    virtual ostream& print(ostream& out) const = 0;
    virtual ~Printable() {};
};

ostream& operator<<(ostream& out, Printable const& p);

struct Node : public Printable
{
    string v_;
    Node *l_, *r_;
    Node(string v) : v_(v), l_(0), r_(0) {}
    ~Node();
    ostream& print(ostream& out) const;
    ostream& print(ostream& out, int depth) const;
};

struct Tree : public Printable
{
    Node* root_;
    ostream& print(ostream& out) const;
    Tree(string infilename);
    ~Tree();
    pair<int, string> infile_line_parser(ifstream& infile);
    void build_tree(ifstream& infile,
                    pair<int,string>& next_nodedat,
                    Node* curr_parent,
                    int curr_depth);
};
