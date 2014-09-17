//  copyright (c) 2014 Grant Mercer
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx_init.hpp>
#include <hpx/hpx.hpp>
#include <hpx/include/parallel_find.hpp>
#include <hpx/util/lightweight_test.hpp>

#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_find_if_not(ExPolicy const& policy, IteratorTag)
{
    BOOST_STATIC_ASSERT(hpx::parallel::is_execution_policy<ExPolicy>::value);

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    //fill vector with random values about 1
    std::fill(boost::begin(c), boost::end(c), (std::rand()%100)+2);
    c.at(c.size()/2) = 1;

    iterator index = hpx::parallel::find_if_not(policy,
        iterator(boost::begin(c)), iterator(boost::end(c)),
        [](std::size_t v) {
            return v != std::size_t(1);
        });

    base_iterator test_index = boost::begin(c) + c.size()/2;

    HPX_TEST(index == iterator(test_index));
}

template <typename ExPolicy, typename IteratorTag>
void test_find_if_not_async(ExPolicy const& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    //fill vector with random values above 1
    std::fill(boost::begin(c), boost::end(c), (std::rand()%100) + 2);
    c.at(c.size()/2) = 1;

    hpx::future<iterator> f =
        hpx::parallel::find_if_not(p,
            iterator(boost::begin(c)), iterator(boost::end(c)),
        [](std::size_t v) {
            return v != std::size_t(1);
        });
    f.wait();

    //create iterator at position of value to be found
    base_iterator test_index = boost::begin(c) + c.size()/2;

    HPX_TEST(f.get() == iterator(test_index));
}

template <typename IteratorTag>
void test_find_if_not()
{
    using namespace hpx::parallel;
    test_find_if_not(seq, IteratorTag());
    test_find_if_not(par, IteratorTag());
    test_find_if_not(par_vec, IteratorTag());

    test_find_if_not_async(seq(task), IteratorTag());
    test_find_if_not_async(par(task), IteratorTag());

    test_find_if_not(execution_policy(seq), IteratorTag());
    test_find_if_not(execution_policy(par), IteratorTag());
    test_find_if_not(execution_policy(par_vec), IteratorTag());

    test_find_if_not(execution_policy(seq(task)), IteratorTag());
    test_find_if_not(execution_policy(par(task)), IteratorTag());
}

void find_if_not_test()
{
    test_find_if_not<std::random_access_iterator_tag>();
    test_find_if_not<std::forward_iterator_tag>();
    test_find_if_not<std::input_iterator_tag>();
}


///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_find_if_not_exception(ExPolicy const& policy, IteratorTag)
{
    BOOST_STATIC_ASSERT(hpx::parallel::is_execution_policy<ExPolicy>::value);

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;
    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), std::rand()+1);
    c[c.size()/2]=0;

    bool caught_exception = false;
    try {
        hpx::parallel::find_if_not(policy,
            decorated_iterator(
                boost::begin(c),
                [](){ throw std::runtime_error("test"); }),
            decorated_iterator(boost::end(c)),
            [](std::size_t v){return 1;});
        HPX_TEST(false);
    }
    catch(hpx::exception_list const& e) {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(policy, e);
    }
    catch(...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag>
void test_find_if_not_exception_async(ExPolicy const& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), std::rand()+1);
    c[c.size()/2]=0;

    bool caught_exception = false;
    try {
        hpx::future<decorated_iterator> f =
            hpx::parallel::find_if_not(p,
                decorated_iterator(
                    boost::begin(c),
                    [](){ throw std::runtime_error("test"); }),
                decorated_iterator(boost::end(c)),
                [](std::size_t v){return 1;});
        f.get();

        HPX_TEST(false);
    }
    catch(hpx::exception_list const& e) {
        caught_exception = true;
        test::test_num_exceptions<ExPolicy, IteratorTag>::call(p, e);
    }
    catch(...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename IteratorTag>
void test_find_if_not_exception()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_find_if_not_exception(seq, IteratorTag());
    test_find_if_not_exception(par, IteratorTag());

    test_find_if_not_exception_async(seq(task), IteratorTag());
    test_find_if_not_exception_async(par(task), IteratorTag());

    test_find_if_not_exception(execution_policy(seq), IteratorTag());
    test_find_if_not_exception(execution_policy(par), IteratorTag());

    test_find_if_not_exception(execution_policy(seq(task)), IteratorTag());
    test_find_if_not_exception(execution_policy(par(task)), IteratorTag());
}

void find_if_not_exception_test()
{
    test_find_if_not_exception<std::random_access_iterator_tag>();
    test_find_if_not_exception<std::forward_iterator_tag>();
    test_find_if_not_exception<std::input_iterator_tag>();
}

//////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_find_if_not_bad_alloc(ExPolicy const& policy, IteratorTag)
{
    BOOST_STATIC_ASSERT(hpx::parallel::is_execution_policy<ExPolicy>::value);

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(100007);
    std::iota(boost::begin(c), boost::end(c), std::rand()+1);
    c[c.size()/2]=0;

    bool caught_bad_alloc = false;
    try {
        hpx::parallel::find_if_not(policy,
            decorated_iterator(
                boost::begin(c),
                [](){ throw std::bad_alloc(); }),
            decorated_iterator(boost::end(c)),
            [](std::size_t v){return 1;});
        HPX_TEST(false);
    }
    catch(std::bad_alloc const&) {
        caught_bad_alloc = true;
    }
    catch(...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
}

template <typename ExPolicy, typename IteratorTag>
void test_find_if_not_bad_alloc_async(ExPolicy const& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(boost::begin(c), boost::end(c), std::rand()+1);
    c[c.size()/2] = 0;

    bool caught_bad_alloc = false;
    try {
        hpx::future<decorated_iterator> f =
            hpx::parallel::find_if_not(p,
                decorated_iterator(
                    boost::begin(c),
                    [](){ throw std::bad_alloc(); }),
                decorated_iterator(boost::end(c)),
                [](std::size_t v){return 1;});

        f.get();

        HPX_TEST(false);
    }
    catch(std::bad_alloc const&) {
        caught_bad_alloc = true;
    }
    catch(...) {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
}

template <typename IteratorTag>
void test_find_if_not_bad_alloc()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_find_if_not_bad_alloc(seq, IteratorTag());
    test_find_if_not_bad_alloc(par, IteratorTag());

    test_find_if_not_bad_alloc_async(seq(task), IteratorTag());
    test_find_if_not_bad_alloc_async(par(task), IteratorTag());

    test_find_if_not_bad_alloc(execution_policy(seq), IteratorTag());
    test_find_if_not_bad_alloc(execution_policy(par), IteratorTag());

    test_find_if_not_bad_alloc(execution_policy(seq(task)), IteratorTag());
    test_find_if_not_bad_alloc(execution_policy(par(task)), IteratorTag());
}

void find_if_not_bad_alloc_test()
{
    test_find_if_not_bad_alloc<std::random_access_iterator_tag>();
    test_find_if_not_bad_alloc<std::forward_iterator_tag>();
    test_find_if_not_bad_alloc<std::input_iterator_tag>();
}

int hpx_main(boost::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int)std::time(0);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    find_if_not_test();
    find_if_not_exception_test();
    find_if_not_bad_alloc_test();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg;
    cfg.push_back("hpx.os_threads=" +
        boost::lexical_cast<std::string>(hpx::threads::hardware_concurrency()));

    HPX_TEST_EQ_MSG(hpx::init(argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
