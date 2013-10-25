void* thread_func(void*)
{
    boost::mutex m;
    boost::mutex::scoped_lock outer(m);
    for (int i=0;i<2;++i) {
        std::cerr << "outer " << i << '\n';
        boost::mutex::scoped_lock inner(m);
        for (int j=0;j<3;++j) {
            std::cerr << "  inner " << j << '\n';
        }
    }
    return NULL;
}
