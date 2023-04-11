#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <stack>
#include <memory>
using namespace std;

int precedence(char op){
    if(op == '^')
        return 3;
    if(op == '*')
        return 2;
    if(op == '+')
        return 1;
    return 0;
}

class Automata{
    shared_ptr<string> q0;
    vector<shared_ptr<string>> F;
    vector<shared_ptr<string>> states;
    map<tuple<shared_ptr<string>, string>, vector<shared_ptr<string>>> delta;

public:
    Automata(const string& l){
        q0 = make_shared<string>("q0");
        states.push_back(q0);
        auto tuple = make_tuple(q0, l);
        auto q1 = make_shared<string>("q1");
        states.push_back(q1);
        delta[tuple].push_back(q1);
        F.push_back(q1);
    }


    void star(){
        shared_ptr<string> newq0 = make_shared<string>("q0prim");
        states.insert(states.begin(), newq0);
        auto tuple = make_tuple(newq0, "~");
        delta[tuple].push_back(q0);
        q0 = newq0;
        for(auto it: F) {
            tuple = make_tuple(it, "~");
            delta[tuple].push_back(q0);
        }
        F.clear();
        F.push_back(q0);
    }

    Automata& operator*(Automata& b){
//        shared_ptr<string> q02 = nullptr;
//        for(auto it: b.delta) {
//            shared_ptr<string> q = make_shared<string>(*get<0>(it.first));
//            if(q02 == nullptr && get<0>(it.first) == b.q0)....
//            string l = get<1>(it.first);
//            auto tuple = make_tuple(q, l);
//            for(auto it2: it.second)
//                delta[tuple].push_back(make_shared<string>(*it2));
//            states.push_back(q)
//        }
        states.insert(states.end(), b.states.begin(), b.states.end());
        delta.merge(b.delta);
        for(auto it: F){
            auto tuple = make_tuple(it, "~");
            delta[tuple].push_back(b.q0);
        }
        F.clear();
        F = b.F;
        return *this;
    }

    Automata& operator+(Automata& b){
        shared_ptr<string> newq0 = make_shared<string>("q0prim");
        states.insert(states.begin(), newq0);
        states.insert(states.end(), b.states.begin(), b.states.end());
        delta.merge(b.delta);
        auto tuple = make_tuple(newq0, "~");
        delta[tuple].push_back(q0);
        delta[tuple].push_back(b.q0);
        q0 = newq0;
        F.insert(F.end(), b.F.begin(), b.F.end());
        return *this;
    }

    void print(){
        cout<<*q0<<"\n";
        for(auto it: F)
            cout<<*it<<" ";
        cout<<"\n";
        for(auto it: delta) {
            cout << *get<0>(it.first) << " " << get<1>(it.first) << " ";
            for(auto it2: it.second)
                cout<<*it2<<" ";
            cout<<"\n";
        }
    }

    void rename(){
        *q0 = "q0";
        int nr = 1;
        for(auto it: states)
            if(it != q0)
                *it = "q" + to_string(nr++);
    }


    ~Automata(){
//        for(auto it: states)
//            delete it;  //trebuie smart pointers??? sau poate le modific valoarea dupa ce ii accesez
    }
};

stack<Automata*> machines;
stack<char> operators;

void apply_op(char op){
    if(op == '*') {
        Automata* b = machines.top();
        machines.pop();
        Automata* a = machines.top();
        machines.pop();
        machines.push(&(*a * *b));   //*a = *a concatenat cu *b, apoi il punem pe stiva
        delete b;    //nu mai avem nevoie de b
        return;
    }
    if(op == '+') {
        Automata* b = machines.top();
        machines.pop();
        Automata* a = machines.top();
        machines.pop();
        machines.push(&(*a + *b));
        delete b;
        return;
    }
    //nu voi mai pune '^' pe stiva operatorilor, ca nu e nevoie. Se poate aplica direct
//    if(op == '^'){
//        Automata* a = machines.top();
//        a->star();
//    }
}

void push_op(char op){
    while(!operators.empty() && precedence(op) <= precedence(operators.top())){
        apply_op(operators.top());
        operators.pop();
    }
    operators.push(op);
}


int main() {
    //algoritm de parsare a unei expresii
    string exp;
    cin>>exp;

    if(exp == "@"){
        cout<<"q0\n@\n@";
        return 0;
    }

    if(exp == "~"){
        cout<<"q0\nq0\n@";
        return 0;
    }

    for(int i=0; i<exp.size(); i++){

        if(exp[i] == '(') {
            if(i > 0 && exp[i-1] != '+' && exp[i-1] != '(')
                operators.push('*');
            operators.push('(');
        }

        else if(exp[i] == ')'){
            while(operators.top() != '('){
                char op = operators.top();
                apply_op(op);
                operators.pop();
            }
            operators.pop();  //sterge '('
        }

        else if(exp[i] == '+')
            push_op('+');

        else if(exp[i] == '^'){
            Automata* a = machines.top();
            a->star();
        }

        else{   //avem o litera
            if(i > 0 && exp[i-1] != '(' && exp[i-1] != '+')
                push_op('*');
            string s(1, exp[i]); //converting a char to a string
            machines.push(new Automata(s));
        }

    }
    while(!operators.empty()){
        char op = operators.top();
        operators.pop();
        apply_op(op);
    }
    machines.top()->rename();
    machines.top()->print();
    delete machines.top();
    return 0;
}
// de dezalocat pointerii din machines --de verificat memory leak
//poate sortez delta dupa nume