// Copyright (c) 2017 Arne Köhn
// All Rights Reserved.
//
// This file is part of TurboParser 2.3.
//
// TurboParser 2.3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.3.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <string>
#include <iostream>
#include "DependencyPipe.h"
#include "TaggerPipe.h"
#include <glog/logging.h>

DependencyPipe *pipedep;
SequencePipe *pipetagger;

string tag(string sentence);
string parse(string sentence);

int main(int argc, const char* argv[])
{
  if (argc != 3) {
	std::cout << "usage: TPinteractive [parser model] [tagger model]" <<endl;
	std::cout <<"write sentences to stdin (one sentence per line), get conll output to stdout." <<endl;
	exit(0);
  }
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "loading models..." << endl;

  //parser
  DependencyOptions *options = new DependencyOptions;
  options->Initialize();
  
  pipedep = new DependencyPipe(options);
  pipedep->Initialize();
  
  pipedep->LoadModelByName(argv[1]);


  //tagger
  TaggerOptions *optionstagger = new TaggerOptions;
  optionstagger->Initialize();

  pipetagger = new TaggerPipe(optionstagger);
  pipetagger->Initialize();

  pipetagger->LoadModelByName(argv[2]);
  LOG(INFO) << "models loaded" << endl;

  for (std::string line; std::getline(std::cin, line);) {
	string tagged = tag(line);
	std::cout << parse(tagged);
  }
  return 0;
}

string toCoarse(string tag) {
  if (tag.find("PRP")==0) {
	  return "PRP";
	} else {
	  return tag.substr(0,2);
	}
  
}

string tag(string sentence) {

  SequenceInstance *instance = new SequenceInstance();

  vector<string> words;
  StringSplit(sentence, " ", &words, true);

  // dummy tags
  string nn = "NN";
  vector<string> tags(words.size(), nn);

  instance->Initialize(words, tags);

  pipetagger->ClassifyInstance(instance);

  string result;

  for (int i = 0; i < instance->size(); ++i) {
    result += to_string(i+1) + "\t";
    result += instance->GetForm(i) + "\t";
    result += instance->GetForm(i) + "\t";
    result += toCoarse(instance->GetTag(i)) + "\t";
    result += instance->GetTag(i) + "\t";
    result += "_\t";
    result += "0\tROOT\t_\t_\n";
  }
  return result;
}

string parse(string sentence) {

  DependencyReader *reader = pipedep->GetDependencyReader();
  DependencyInstance *instance = reader->StringToInstance(sentence);
  pipedep->ClassifyInstance(instance);

  string result;

  for (int i = 1; i < instance->size(); ++i) {
    result += to_string(i) + "\t";
    result += instance->GetForm(i) + "\t";
    result += instance->GetLemma(i) + "\t";
    result += instance->GetCoarsePosTag(i) + "\t";
    result += instance->GetPosTag(i) ;
    result += "\t_\t";
    result += to_string(instance->GetHead(i)) + "\t";
    result += instance->GetDependencyRelation(i);
	result.append("\n");
  }

  return result;
}
