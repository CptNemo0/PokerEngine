#ifndef COMMON_UTILITY_SCOPED_OBSERVATION_H_
#define COMMON_UTILITY_SCOPED_OBSERVATION_H_

#include <cassert>

namespace common::utility {

template <class Observable, class Observer>
class ScopedObservation {
    static_assert(requires(Observable& observable, Observer* observer) {
      observable.AddObserver(observer);
      observable.RemoveObserver(observer);
    });

  public:
    explicit ScopedObservation(Observer* observer) : observer_(observer) {
    }

    ScopedObservation(const ScopedObservation&) = delete;
    void operator=(const ScopedObservation&) = delete;

    ~ScopedObservation() {
      Reset();
    }

    void Observe(Observable* observable) {
      assert(!observable_ && observable && observer_);
      observable_ = observable;
      observable_->AddObserver(observer_);
    }

    void Reset() {
      assert(observable_ && observer_);
      observable_->RemoveObserver(observer_);
      observable_ = nullptr;
    }

    const Observer* GetObserver() const {
      return observer_;
    }

    const Observable* GetObservable() const {
      return observable_;
    }

  private:
    Observer* observer_;
    Observable* observable_{nullptr};
};

} // namespace common::utility

#endif // !COMMON_UTILITY_SCOPED_OBSERVATION_H_
