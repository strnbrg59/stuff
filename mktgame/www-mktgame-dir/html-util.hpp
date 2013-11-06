struct Html_argument {
	String name;
	String value;
};

class Html_argument_array {
// Associative array of Html_argument structs.

	Html_argument* rep;
	char* stdin_buf;
	int len;
  public:
	Html_argument_array( int );
	Html_argument_array();
	~Html_argument_array();

	int n_args();
	void show_stdin();

	String operator[]( String name ); // retrieves value, given name.
	String operator[]( int i ); 	  // retrieves value.

	String name( int i ); // returns rep[i].name
};
