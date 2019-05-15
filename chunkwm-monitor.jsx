export const command = "/Users/shsiang/Documents/CS/desktop/monitor"

export const refreshFrequency = 1000

export const render = ({ output }) => {
  return (
    <div className="screen">
      <div className="chunkwm-monitor">{ output }</div>
    </div>
  )
}
