/*
 * File: fixt.h
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: High-level global interface to the scheduling test fixture
 */

#ifndef FIXT_H_
#define FIXT_H_

/**
 * The test fixture will run individual tests (which are infinite in nature)
 * for this number of seconds before cancelling the test.
 */
#define FIXT_SECONDS_PER_TEST 1

/*
 * Initialize the test fixture (globally).
 */
void fixt_init();

/*
 * Begin testing.
 */
void fixt_test();

/**
 * Terminate testing and free up resources created while testing.
 */
void fixt_term();

#endif
