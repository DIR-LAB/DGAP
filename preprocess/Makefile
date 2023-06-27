all:
	g++ -o adj_to_el_converter adj_to_el_converter.cc -std=c++11 -O3
	g++ -o create_llama_dataset create_llama_dataset.cc -std=c++11 -O3
	g++ -o shuffle_dataset shuffle_dataset.cc -std=c++11 -O3
	g++ -o split_dataset split_dataset.cc -std=c++11 -O3
	g++ -o text2bin text_to_bin.cpp -std=c++11 -O3
	g++ -o weighted_to_unweighted_conversion weighted_to_unweighted_conversion.cc -std=c++11 -O3

.PHONY: clean
clean:
	rm -f adj_to_el_converter create_llama_dataset shuffle_dataset split_dataset text2bin weighted_to_unweighted_conversion
