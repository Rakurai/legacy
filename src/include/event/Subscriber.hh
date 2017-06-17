namespace event {

class Subscriber {
public:
	virtual ~Subscriber() {}

	virtual void notify(Event&) = 0;

private:
	Subscriber();
	Subscriber(const Subscriber&);
	Subscriber& operator=(const Subscriber&);
};

} // namespace event
