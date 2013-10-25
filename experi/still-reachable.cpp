struct MS {
    MS* next;
};

int main()
{
    MS* ms1 = new MS;
    ms1->next = new MS;
    delete ms1->next;
    delete ms1;
}
