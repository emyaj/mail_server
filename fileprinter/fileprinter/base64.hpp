//
//  base64.hpp
//  fileprinter
//
//  Created by Jayme Cartwright on 4/25/17.
//  Copyright © 2017 Jayme Cartwright. All rights reserved.
//

#ifndef base64_hpp
#define base64_hpp

#include <stdio.h>
#include <string>

using namespace std;

string base64_encode(unsigned char const* , unsigned int len);
string base64_decode(std::string const& s);


#endif /* base64_hpp */
