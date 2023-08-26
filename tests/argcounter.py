#!/usr/bin/env python3

import sys

def main():
    for i, arg in enumerate(sys.argv):
        print(f'arg[{i}]: {arg}')

if __name__ == '__main__':
    main()
