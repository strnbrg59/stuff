Program LinkedList;
Type
    node = record
        num :integer;
        next :^node;
    p :^node;
End;
Var
    ptr :^node;
    start :^node;
    last :^node;

Procedure createlist;
Var
    i :integer;
Begin
    new(start);
    new(ptr);
    start^.num := 1;
    ptr := start;
    for i := 2 to 5 do
    begin
        new(last);
        last^.num := i;
        ptr^.next := last;
        ptr := last;
    end;
    ptr^.next := NIL;
End;

Procedure Print;
Begin
    ptr := start;
    while ptr <> NIL do
    begin
        writeln(ptr^.num);
        ptr := ptr^.next;
    end;
End;

Procedure prepend(value : Integer);
Var
    newrecord :^node;
begin
    new(newrecord);
    newrecord^.num := value;
    newrecord^.next := start;
    start := newrecord;
end;

Procedure insert(afternum : Integer);
Var
    newrecord :^node;
Begin
    new (newrecord);
    newrecord^.num := 45;
    ptr := start;
    while ptr^.num <> afternum do
    begin
        ptr := ptr^.next;  
    end;
    newrecord^.next := ptr^.next;
    ptr^.next := newrecord;
End;
    
Begin
CreateList;
prepend(99);
prepend(-69);
insert(1);
insert(45);
Print;
End.
