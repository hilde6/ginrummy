#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <chrono>
#include <random>
using namespace std;

struct playing_card
{
   bool operator==(const playing_card& rhs) const { return this->value == rhs.value; }
   bool operator>(const playing_card& rhs) const { return this->value > rhs.value; }
   bool operator<(const playing_card& rhs) const { return this->value < rhs.value; }

   char suit;               
   char card;                
   int value;
};

constexpr size_t NSUITS=4;
constexpr size_t NCARDS_PER_SUIT=13;
constexpr size_t NCARDS=52;
constexpr size_t NHAND=10;
constexpr size_t MAXSCORE=50;
constexpr size_t NGAMES=10000;
// constexpr size_t NGAMES=1;

bool print_final=false;

string player_1_name = "Janbot";
string player_2_name = "Dumbbot";

void init(vector<playing_card>& deck)
{
   static const char suits[NSUITS] = { 'S', 'H', 'D', 'C' } ;
   static constexpr char letters[NCARDS_PER_SUIT+1] = "A23456789TJQK" ;

   for( size_t index = 0 ; index < NCARDS ; ++index )
   {
      deck.push_back(playing_card());
      deck[index].suit=suits[ index / NCARDS_PER_SUIT ];
      deck[index].value=index % NCARDS_PER_SUIT + 1;
      deck[index].card=letters[index % NCARDS_PER_SUIT];
   }
}

void print( const playing_card& card ) { cout << card.card << "_" << card.suit <<  ' ' ; }

void print( const vector<playing_card>& deck )
{
   for( size_t i = 0; i < deck.size(); ++i) print(deck[i]) ;
   cout << "\n-------------------------\n" ;
}

void print( const vector<vector<playing_card>>& deck )
{
   for(size_t i = 0;i<deck.size();++i){
      for(size_t j=0;j<deck[i].size();++j) print(deck[i][j]);
   // cout << "\n-------------------------\n" ;
   }
   cout << "\n-------------------------\n" ;
}

void shuffle( vector<playing_card>& deck )
{
   unsigned seed = chrono::system_clock::now().time_since_epoch().count();
   shuffle(deck.begin(),deck.end(),default_random_engine(seed));
}

template<typename T>
vector<T> flatten_v(const vector<vector<T>>& v2d)
{
   vector<T> v1d;
   for(const auto &v: v2d)
      v1d.insert(v1d.end(),v.begin(),v.end());
   return v1d;
}

playing_card draw(vector<playing_card>& deck){
   if(deck.empty()) cout<<"Error: drawing from empty deck!" <<endl;
   playing_card card = deck[0];
   deck.erase(deck.begin());
   return card;
}

int card_value(const playing_card& card){
   if(card.value>10) return 10;
   return card.value;
}

int deadwood(const vector<playing_card>& hand)
{
   if(hand.empty()) return 0;
   int sum=0;
   for(const auto &h: hand) sum+=card_value(h);
   return sum;
}

bool suitsort(const playing_card& a, const playing_card& b){
   int a_n,b_n;
   switch(a.suit){
      case 'C':
         a_n=1;
         break;
      case 'D':
         a_n=2;
         break;
      case 'H':
         a_n=3;
         break;
      case 'S':
         a_n=4;
         break;
      default:
         a_n=0;
   }
   switch(b.suit){
      case 'C':
         b_n=1;
         break;
      case 'D':
         b_n=2;
         break;
      case 'H':
         b_n=3;
         break;
      case 'S':
         b_n=4;
         break;
      default:
         b_n=0;
   }
   
   // mutiply by max value to ensure suits are sorted first
   return 15*a_n+a.value<15*b_n+b.value;
}

vector<vector<int>> findpairs(const vector<playing_card>& hand)
{
   vector<vector<int>> use_index;
   vector<int> exp_index;
   vector<int> tmp_index;
   int n_hand=hand.size();
   for(size_t i=0;i<n_hand;++i){
      tmp_index.clear();
      int count=1;
      vector<int>::iterator it=find(exp_index.begin(),exp_index.end(),i);
      if(it==exp_index.end()){ // check this index has not already been used
         for(size_t j=i+1;j<n_hand;++j){
            if(hand[j]==hand[i]){
               if(count==1) tmp_index.push_back(i);
               count++;
               tmp_index.push_back(j);
            }
         }
         if(count>2) {
            for(size_t k=0;k<count;++k) exp_index.push_back(tmp_index[k]);
            use_index.push_back(tmp_index);
         }
      }
   }

   exp_index.clear();
   tmp_index.clear();

   return use_index;
}

vector<vector<int>> findstraights(vector<playing_card>& hand)
{
   vector<vector<int>> use_index;
   vector<int> tmp_index;
   int n_hand=hand.size();
   sort(hand.begin(),hand.end(),suitsort);

   for(size_t i=0;i<n_hand;++i){
      tmp_index.clear();
      size_t j=i+1;
      while(j<n_hand && hand[j].suit==hand[i].suit && hand[j].value==hand[i].value+1+tmp_index.size()){
         tmp_index.push_back(j);
         j++;
      }
      if(tmp_index.size()>1){
         tmp_index.insert(tmp_index.begin(),i);
         use_index.push_back(tmp_index);
         // use_index.push_back(i);
         // for(size_t k=0;k<j-i-1;++k) use_index.push_back(tmp_index[k]);
         i=j;
      }
   }

   tmp_index.clear();

   return use_index;
}

void find_sets(vector<playing_card>& hand, vector<vector<playing_card>>& used, vector<playing_card>& left)
{
   int n_hand = hand.size();

   //Check for straights then pairs
   vector<vector<int>> use_index_s=findstraights(hand);
   vector<int> exp_index_s=flatten_v(use_index_s);
   vector<playing_card> tmp_used;
   for(size_t i=0;i<use_index_s.size();++i){
      tmp_used.clear();
      for(size_t j=0;j<use_index_s[i].size();++j){
         tmp_used.push_back(hand[use_index_s[i][j]]);
      }
      used.push_back(tmp_used);
   }
   for(size_t i=0;i<n_hand;++i){
      vector<int>::iterator it=find(exp_index_s.begin(),exp_index_s.end(),i);
      if(it==exp_index_s.end()) left.push_back(hand[i]);
   }

   vector<vector<int>> use_index_p=findpairs(left);
   vector<int> exp_index_p=flatten_v(use_index_p);
   for(size_t i=0;i<use_index_p.size();++i){
      tmp_used.clear();
      for(size_t j=0;j<use_index_p[i].size();++j){
         tmp_used.push_back(left[use_index_p[i][j]]);
      }
      used.push_back(tmp_used);
   }
   int count=0;
   int N=left.size();
   for(size_t i=0;i<N;++i){
      vector<int>::iterator it=find(exp_index_p.begin(),exp_index_p.end(),i);
      if(it!=exp_index_p.end()){ 
         left.erase(left.begin()+i-count);
         count++;
      }
   }

   //Check if pairs then straights are better
   vector<vector<playing_card>> used_p;
   vector<playing_card> left_p;
   use_index_p.clear();
   use_index_s.clear();
   exp_index_p.clear();
   exp_index_s.clear();
   use_index_p=findpairs(hand);
   exp_index_p=flatten_v(use_index_p);
   for(size_t i=0;i<use_index_p.size();++i){
      tmp_used.clear();
      for(size_t j=0;j<use_index_p[i].size();++j){
         tmp_used.push_back(hand[use_index_p[i][j]]);
      }
      used_p.push_back(tmp_used);
   }
   for(size_t i=0;i<n_hand;++i){
      vector<int>::iterator it=find(exp_index_p.begin(),exp_index_p.end(),i);
      if(it==exp_index_p.end()) left_p.push_back(hand[i]);
   }

   use_index_s=findstraights(left_p);
   exp_index_s=flatten_v(use_index_s);
   for(size_t i=0;i<use_index_s.size();++i){
      tmp_used.clear();
      for(size_t j=0;j<use_index_s[i].size();++j){
         tmp_used.push_back(left_p[use_index_s[i][j]]);
      }
      used_p.push_back(tmp_used);
   }
   count=0;
   N=left_p.size();
   for(size_t i=0;i<N;++i){
      vector<int>::iterator it=find(exp_index_s.begin(),exp_index_s.end(),i);
      if(it!=exp_index_s.end()){ 
         left_p.erase(left_p.begin()+i-count);
         count++;
      }
   }

   if(deadwood(left_p)<deadwood(left)){
      left.swap(left_p);
      used.swap(used_p);
   }

   left_p.clear();
   used_p.clear();
   tmp_used.clear();
   use_index_p.clear();
   use_index_s.clear();
   exp_index_p.clear();
   exp_index_s.clear();

}

void layoff(vector<vector<playing_card>>& used, vector<playing_card>& left)
{
   sort(left.begin(),left.end(),suitsort);
   vector<int> use_index;
   //first find all matches with straights
   for(size_t i=0;i<left.size();++i){
      for(size_t j=0;j<used.size();++j){
         if(used[j][0].suit!=used[j][1].suit) break; //check if this used set is a straight
         if(left[i].suit==used[j][used[j].size()].suit && left[i].value==used[j][used[j].size()].value+1){
            use_index.push_back(i);
            if(i+1<left.size() && left[i+1].suit==left[i].suit && left[i+1].value==left[i].value+1){
               use_index.push_back(i+1); 
               i++;
            }
            break;
         }else if(left[i].suit==used[j][0].suit && left[i].value==used[j][0].value-1){
            use_index.push_back(i);
            if(i-1>=0 && left[i-1].suit==left[i].suit && left[i-1].value==left[i].value-1) use_index.push_back(i-1);
            break;
         }
      }
   }

   //delete cards we matched with staights
   int count=0;
   for(size_t i=0;i<left.size();++i){
      vector<int>::iterator it=find(use_index.begin(),use_index.end(),i);
      if(it!=use_index.end()){ 
         left.erase(left.begin()+i-count);
         count++;
      }
   }

   //find pairs
   use_index.clear();
   for(size_t i=0;i<left.size();++i){
      for(size_t j=0;j<used.size();++j){
         if(left[i]==used[j][0] && used[j][0]==used[j][1]){
            use_index.push_back(i);
            break;
         }
      }
   }

   count=0;
   for(size_t i=0;i<left.size();++i){
      vector<int>::iterator it=find(use_index.begin(),use_index.end(),i);
      if(it!=use_index.end()){ 
         left.erase(left.begin()+i-count);
         count++;
      }
   }

   use_index.clear();

}

void calc_scores(int& who_starts, int scores[], vector<vector<playing_card>> used1, vector<playing_card> left1, vector<vector<playing_card>> used2, vector<playing_card> left2, bool play1, bool play2)
{
   if(play1 == false){
      if(left1.empty())
      {
         scores[0]+=25+deadwood(left2);
         who_starts=1;
      }else{
         layoff(used1,left2);
         if(deadwood(left1)<deadwood(left2)){
            scores[0]+=deadwood(left2)-deadwood(left1);
            who_starts=1;
         }else{
            scores[1]+=25+deadwood(left1)-deadwood(left2);
            who_starts=2;
         }
      }
   }else{
      if(left2.empty())
      {
         scores[1]+=25+deadwood(left1);
         who_starts=2;
      }else{
         layoff(used2,left1);
         if(deadwood(left2)<deadwood(left1)){
            scores[1]+=deadwood(left1)-deadwood(left2);
            who_starts=2;
         }else{
            scores[0]+=25+deadwood(left2)-deadwood(left1);
            who_starts=1;
         }
      }
   }
}

// vector<int> get_values(vector<playing_card>& hand)
// {
//    vector<int> values;
//    sort(hand.begin(),hand.end(),suitsort);

//    for(size_t i=0;i<hand.size();++i)
//    {
//       if(hand[i].suit==used[j][used[j].size()].suit && left[i].value==hand[j][used[j].size()].value+1){
//    }

// }

void dumbbot(vector<vector<playing_card>>& used, vector<playing_card>& left, vector<playing_card>& hand, vector<playing_card>& discard, vector<playing_card>& deck, bool& play, bool& first)
{
   if(first || (used.empty() && left.empty())){
      //called for the first move only
      find_sets(hand,used,left); 
   }else{ //we need to recombine our hand to check if discard is better
      hand.clear();
      vector<playing_card> used_f=flatten_v(used);
      hand.reserve(used_f.size()+left.size());
      hand.insert(hand.end(),used_f.begin(),used_f.end());
      hand.insert(hand.end(),left.begin(),left.end());
   }

   sort(hand.begin(),hand.end());

   //Check if using discard creates better hand
   vector<playing_card> hand_dc(hand);
   vector<playing_card> left_dc;
   vector<vector<playing_card>> used_dc;

   hand_dc.push_back(discard.back());
   find_sets(hand_dc,used_dc,left_dc);

   sort(left.begin(),left.end());

   if(deadwood(left_dc)-card_value(left.back())<deadwood(left)){
      left.swap(left_dc);
      used.swap(used_dc);
      discard.erase(discard.end()-1);
      first=false;
   }else{ //Not taking discard, therefore pass or draw a card
      if(first) return;
      hand.push_back(draw(deck));
      used.clear();
      left.clear();
      find_sets(hand,used,left);
   }

   //Check for a gin
   if(left.empty() && !first){
      play=false;
      return;
   }

   //Discard highest value card not being used
   sort(left.begin(),left.end());
   discard.push_back(left.back());
   left.erase(left.end()-1);

   //if you can knock, just do it
   if(deadwood(left)<=10 && !first){
      play=false;
   }

   hand_dc.clear();
   left_dc.clear();
   used_dc.clear();
}

void janbot(vector<vector<playing_card>>& used, vector<playing_card>& left, vector<playing_card>& hand, vector<playing_card>& discard, vector<playing_card>& deck, bool& play, bool& first)
{
   bool print_game=false;

   if(first || (used.empty() && left.empty())){
      //called for the first move only
      find_sets(hand,used,left); 
   }else{ //we need to recombine our hand to check if discard is better
      hand.clear();
      vector<playing_card> used_f=flatten_v(used);
      hand.reserve(used_f.size()+left.size());
      hand.insert(hand.end(),used_f.begin(),used_f.end());
      hand.insert(hand.end(),left.begin(),left.end());
   }

   sort(hand.begin(),hand.end());

   if(print_game){
      cout<<"Starting hand: ";
      print(hand);
      cout<<"Discard pile: ";
      print(discard);
   }

   //Check if using discard creates better hand
   vector<playing_card> hand_dc(hand);
   vector<playing_card> left_dc;
   vector<vector<playing_card>> used_dc;

   hand_dc.push_back(discard.back());
   find_sets(hand_dc,used_dc,left_dc);

   sort(left.begin(),left.end());

   if(deadwood(left_dc)-card_value(left.back())<deadwood(left)){
      left.swap(left_dc);
      used.swap(used_dc);
      discard.erase(discard.end()-1);
      first=false;
   }else{ //Not taking discard, therefore draw a card
      if(first){
         if(print_game) cout<<"Passing\n-------------------------\n"; 
         return;
      }
      if(print_game){
         cout<<"Drawing card: ";
         print(deck[0]);
         cout << "\n-------------------------\n" ;
      }
      hand.push_back(draw(deck));
      used.clear();
      left.clear();
      find_sets(hand,used,left);
   }

   //Check for a gin
   if(left.empty() && !first){
      play=false;
      return;
   }

   //Discard highest value card not being used
   // vector<int> left_values = get_values(left);
   sort(left.begin(),left.end());
   discard.push_back(left.back());
   left.erase(left.end()-1);

   //knocking conditions
   if((deadwood(left)<=10 && deck.size()>28) || (deadwood(left)<=5 && deck.size()>23) || (deadwood(left)<=2 && deck.size()>18) && !first){
      play=false;
   }

   if(print_game){
      cout<<"Used cards: ";
      print(used);
      cout<<"Left cards: ";
      print(left);
      cout<<"Discard pile: ";
      print(discard);
      cout<<"play = "<<play<<endl;
      cout<<endl;
   }

   hand_dc.clear();
   left_dc.clear();
   used_dc.clear();
}

void play_rummy(int scores[], int& who_starts, vector<playing_card>& deck)
{
   vector<playing_card> hand1,hand2;
   for (size_t i=0;i<NHAND;++i)
   {
      hand1.push_back(deck[0]);
      deck.erase(deck.begin());
   }
   for (size_t i=0;i<NHAND;++i)
   {
      hand2.push_back(deck[0]);
      deck.erase(deck.begin());
   }

   vector<playing_card> discard;
   discard.push_back(draw(deck));

   vector<vector<playing_card>> used1,used2;
   vector<playing_card> left1,left2;

   bool play1 = true;
   bool play2 = true;
   bool first = true;

   if(who_starts==1){
      while(play2 == true && !deck.empty()){
         janbot(used1,left1,hand1,discard,deck,play1,first);
         if(play1==false || deck.empty()) break;
         dumbbot(used2,left2,hand2,discard,deck,play2,first);
         first=false;
      }
   }else{
      while(play1 == true && !deck.empty()){
         dumbbot(used2,left2,hand2,discard,deck,play2,first);
         if(play2==false || deck.empty()) break;
         janbot(used1,left1,hand1,discard,deck,play1,first);
         first=false;
      }
   }

   if(print_final){
      cout<<player_1_name+" used:  ";
      print(used1);
      cout<<player_1_name+" left:  ";
      print(left1);
      cout<<player_2_name+" used:  ";
      print(used2);
      cout<<player_2_name+" left:  ";
      print(left2);
   }

   if(!deck.empty()) calc_scores(who_starts,scores,used1,left1,used2,left2,play1,play2);
   if(print_final) cout<<player_1_name+" score: "<<scores[0]<<", "+player_2_name+" score: "<<scores[1]<<endl;

   hand1.clear();
   hand2.clear();
   used1.clear();
   used2.clear();
   left1.clear();
   left2.clear();
   discard.clear();
}

int main()
{
   srand(time(nullptr));

   using std::chrono::high_resolution_clock;
   using std::chrono::duration_cast;
   using std::chrono::duration;
   using std::chrono::milliseconds;

   vector<playing_card> my_deck;

   size_t win_count=0;

   auto t1=high_resolution_clock::now();

   for(size_t i=0;i<NGAMES;++i){

      int scores[2]={0,0};

      //choose if player 1 or 2 starts first
      int who_starts = rand()%2+1;

      while(scores[0]<MAXSCORE && scores[1]<MAXSCORE){
         my_deck.clear();
         init(my_deck);
         shuffle(my_deck);
         play_rummy(scores,who_starts,my_deck);
         if(print_final) cout << "===============================================================\n";
      }
      if(scores[0]>scores[1]){
         if(print_final) cout<<player_1_name+" wins!\n"<<endl;
         win_count++;
      }else{
         if(print_final) cout<<player_2_name+" wins!\n"<<endl;
      }

      my_deck.clear();
   }

   auto t2=high_resolution_clock::now();

   auto time=duration_cast<milliseconds>(t2-t1);

   cout<<player_1_name+" won "<<win_count<<" games, "+player_2_name+" won "<<NGAMES-win_count<<" games."<<endl;
   cout<<"Total time = "<<time.count()*1.0e-3<<" seconds"<<endl;

}
