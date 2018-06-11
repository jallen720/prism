#pragma once

namespace prism
{

template<typename T>
struct Container
{
    T * data;
    size_t count;
};

template<typename T>
Container<T>
createContainer(size_t count)
{
    Container<T> container = {};
    container.data = (T *)malloc(sizeof(T) * count);
    container.count = count;
    return container;
}

template<typename T>
void
freeContainer(const Container<T> * container)
{
    free(container->data);
}

} // namespace prism
