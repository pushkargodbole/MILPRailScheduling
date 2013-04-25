#include <iostream>
#include <vector>
#include <string>

using namespace std;

template <class datatype_ref, class datatype_sort>
vector<datatype_sort> mergesort_lookup(vector<datatype_ref> ref, vector<datatype_sort> sort)
{
	int n = ref.size();
	int m = sort.size();
	if(m==n)
	{
		if(n > 1)
		{
			vector<datatype_ref> subarray1_ref;
			vector<datatype_ref> subarray2_ref;
			vector<datatype_sort> subarray1_sort;
			vector<datatype_sort> subarray2_sort;
			vector<datatype_sort> merged_vec;
			for(int i=0;i<n/2;i++)
			{
				subarray1_ref.push_back(ref[i]);
				subarray2_ref.push_back(ref[n/2+i]);
				subarray1_sort.push_back(sort[i]);
				subarray2_sort.push_back(sort[n/2+i]);
			}
			if(n%2 != 0)
			{
				subarray2_ref.push_back(ref[n-1]);
				subarray2_sort.push_back(sort[n-1]);
			}
			subarray1_sort = mergesort_lookup(subarray1_ref,subarray1_sort);
			subarray2_sort = mergesort_lookup(subarray2_ref,subarray2_sort);
			subarray1_ref = mergesort_lookup(subarray1_ref,subarray1_ref);
			subarray2_ref = mergesort_lookup(subarray2_ref,subarray2_ref);
			merged_vec = merge(subarray1_ref, subarray2_ref, subarray1_sort, subarray2_sort);
			subarray1_ref.clear();
			subarray2_ref.clear();
			subarray1_sort.clear();
			subarray2_sort.clear();
			return merged_vec;
		}
		else if(n==1) return sort;
	}
	else cout << "Megresort! : Lengths of the two vectors dont match" << endl;
}

template <class datatype_ref, class datatype_sort>
vector<datatype_sort> merge(vector<datatype_ref> array1_ref,vector<datatype_ref> array2_ref,vector<datatype_sort> array1_sort,vector<datatype_sort> array2_sort)
{
	int array1_count = 0;
	int array2_count = 0;
	vector<datatype_sort> merged_vector;
	while(array1_count < array1_ref.size() && array2_count < array2_ref.size())
	{
		if(array1_ref[array1_count] < array2_ref[array2_count])
		{
			merged_vector.push_back(array1_sort[array1_count]);
			array1_count++;
		}
		else
		{
			merged_vector.push_back(array2_sort[array2_count]);
			array2_count++;
		}
	}
	if(array1_count < array1_ref.size())
	{
		while(array1_count < array1_ref.size())
		{
			merged_vector.push_back(array1_sort[array1_count]);
			array1_count++;
		}
	}
	else if(array2_count < array2_ref.size())
	{
		while(array2_count < array2_ref.size())
		{
			merged_vector.push_back(array2_sort[array2_count]);
			array2_count++;
		}
	}
	return merged_vector;
}

/*int main()
{
	int a_refrray[] = {3,2,5,1,4};
	string b_refrray[] = {"is ","name ","Gonsavlis! :)","My ","Anthony "};
	vector<int> a_vec(a_refrray, a_refrray + sizeof(a_refrray)/sizeof(int));
	vector<string> b_vec(b_refrray, b_refrray + sizeof(b_refrray)/sizeof(string));
	b_vec = mergesort_lookup <int, string> (a_vec, b_vec);
	for(int i=0;i<a_vec.size();i++) cout << b_vec[i] << " ";
	cout << endl;
	for(int i=0;i<a_vec.size();i++) cout << a_vec[i] << " ";
	cout << endl;
}*/
