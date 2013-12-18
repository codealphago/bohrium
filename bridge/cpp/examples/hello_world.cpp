/*
This file is part of Bohrium and Copyright (c) 2012 the Bohrium team:
http://bohrium.bitbucket.org

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with bohrium. 

If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <complex>
#include "bh/bh.hpp"

using namespace bh;

template <typename T>
void complex_ones()
{
    multi_array<std::complex<T> > cc;
    multi_array<T> r, i;

    cc      = ones<std::complex<T> >(3,3);
    r       = real<std::complex<T>, T>(cc);
    i       = imag<std::complex<T>, T>(cc);

    std::cout << r << "111" << i << "222" << cc << "!!!" << std::endl;
}

template <typename T>
void complex_constant()
{
    multi_array<std::complex<T> > cc;
    multi_array<T> r, i;

    cc      = ones<std::complex<T> >(3,3);
    cc      = (T)4.5;
    r       = real<std::complex<T>, T>(cc);
    i       = imag<std::complex<T>, T>(cc);

    std::cout << r << "111" << i << "222" << cc << "!!!" << std::endl;
}

void compute()
{
    std::cout << "Hello World." << std::endl;

    /*
    complex_ones<double>();
    complex_ones<float>();

    complex_constant<double>();
    complex_constant<float>();
    */

    multi_array<float> a;
    a = ones<float>(3,3,3); 

    //std::cout << reduce(a, ADD, 0) << std::endl;
    std::cout << scan(a, SUM, 0) << std::endl;
}

int main()
{
    compute();
    return 0;
}

